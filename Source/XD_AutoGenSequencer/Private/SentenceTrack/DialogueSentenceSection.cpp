// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueSentenceSection.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "AudioDevice.h"

DECLARE_CYCLE_STAT(TEXT("Dialogue Sentence Track Evaluate"), MovieSceneEval_DialogueSentenceTrack_Evaluate, STATGROUP_MovieSceneEval);
DECLARE_CYCLE_STAT(TEXT("Dialogue Sentence Track Tear Down"), MovieSceneEval_DialogueSentenceTrack_TearDown, STATGROUP_MovieSceneEval);
DECLARE_CYCLE_STAT(TEXT("Dialogue Sentence Track Token Execute"), MovieSceneEval_DialogueSentenceTrack_TokenExecute, STATGROUP_MovieSceneEval);

/** Stop audio from playing */
struct FStopDialogueSentencePreAnimatedToken : IMovieScenePreAnimatedToken
{
	static FMovieSceneAnimTypeID GetAnimTypeID()
	{
		return TMovieSceneAnimTypeID<FStopDialogueSentencePreAnimatedToken>();
	}

	virtual void RestoreState(UObject& InObject, IMovieScenePlayer& Player) override
	{
		UAudioComponent* AudioComponent = CastChecked<UAudioComponent>(&InObject);
		if (AudioComponent)
		{
			AudioComponent->Stop();
			AudioComponent->DestroyComponent();
		}
	}

	struct FProducer : IMovieScenePreAnimatedTokenProducer
	{
		virtual IMovieScenePreAnimatedTokenPtr CacheExistingState(UObject& Object) const override
		{
			return FStopDialogueSentencePreAnimatedToken();
		}
	};
};

/** Destroy a transient audio component */
struct FDestroyDialogueSentencePreAnimatedToken : IMovieScenePreAnimatedToken
{
	static FMovieSceneAnimTypeID GetAnimTypeID()
	{
		return TMovieSceneAnimTypeID<FDestroyDialogueSentencePreAnimatedToken>();
	}

	virtual void RestoreState(UObject& InObject, IMovieScenePlayer& Player) override
	{
		UAudioComponent* AudioComponent = CastChecked<UAudioComponent>(&InObject);
		if (AudioComponent)
		{
			AudioComponent->DestroyComponent();
		}
	}

	struct FProducer : IMovieScenePreAnimatedTokenProducer
	{
		virtual IMovieScenePreAnimatedTokenPtr CacheExistingState(UObject& Object) const override
		{
			return FDestroyDialogueSentencePreAnimatedToken();
		}
	};
};

struct FCachedDialogueSentenceTrackData : IPersistentEvaluationData
{
	TMap<FObjectKey, TMap<FObjectKey, TWeakObjectPtr<UAudioComponent>>> AudioComponentsByActorKey;

	FCachedDialogueSentenceTrackData()
	{
		// Create the container for master tracks, which do not have an actor to attach to
		AudioComponentsByActorKey.Add(FObjectKey(), TMap<FObjectKey, TWeakObjectPtr<UAudioComponent>>());
	}

	UAudioComponent* GetAudioComponent(FObjectKey ActorKey, FObjectKey SectionKey)
	{
		if (TMap<FObjectKey, TWeakObjectPtr<UAudioComponent>> * Map = AudioComponentsByActorKey.Find(ActorKey))
		{
			// First, check for an exact match for this section
			TWeakObjectPtr<UAudioComponent> ExistingComponentPtr = Map->FindRef(SectionKey);
			if (ExistingComponentPtr.IsValid())
			{
				return ExistingComponentPtr.Get();
			}

			// If no exact match, check for any AudioComponent that isn't busy
			for (TPair<FObjectKey, TWeakObjectPtr<UAudioComponent >> Pair : *Map)
			{
				UAudioComponent* ExistingComponent = Map->FindRef(Pair.Key).Get();
				if (ExistingComponent && !ExistingComponent->IsPlaying())
				{
					// Replace this entry with the new SectionKey to claim it
					Map->Remove(Pair.Key);
					Map->Add(SectionKey, ExistingComponent);
					return ExistingComponent;
				}
			}
		}

		return nullptr;
	}

	/** Only to be called on the game thread */
	UAudioComponent* AddAudioComponentForRow(int32 RowIndex, FObjectKey SectionKey, AActor& PrincipalActor, IMovieScenePlayer& Player)
	{
		FObjectKey ActorKey(&PrincipalActor);

		if (!AudioComponentsByActorKey.Contains(ActorKey))
		{
			AudioComponentsByActorKey.Add(ActorKey, TMap<FObjectKey, TWeakObjectPtr<UAudioComponent>>());
		}

		UAudioComponent* ExistingComponent = GetAudioComponent(ActorKey, SectionKey);
		if (!ExistingComponent)
		{
			USoundCue* TempPlaybackAudioCue = NewObject<USoundCue>();

			FAudioDevice::FCreateComponentParams Params(PrincipalActor.GetWorld(), &PrincipalActor);
			ExistingComponent = FAudioDevice::CreateComponent(TempPlaybackAudioCue, Params);

			if (!ExistingComponent)
			{
				FString ActorName =
#if WITH_EDITOR
					PrincipalActor.GetActorLabel();
#else
					PrincipalActor.GetName();
#endif
				UE_LOG(LogMovieScene, Warning, TEXT("Failed to create audio component for spatialized audio track (row %d on %s)."), RowIndex, *ActorName);
				return nullptr;
			}

			Player.SavePreAnimatedState(*ExistingComponent, FMovieSceneAnimTypeID::Unique(), FDestroyDialogueSentencePreAnimatedToken::FProducer());

			AudioComponentsByActorKey[ActorKey].Add(SectionKey, ExistingComponent);

			ExistingComponent->SetFlags(RF_Transient);
			ExistingComponent->AttachToComponent(PrincipalActor.GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		}

		return ExistingComponent;
	}

	/** Only to be called on the game thread */
	UAudioComponent* AddMasterAudioComponentForRow(int32 RowIndex, FObjectKey SectionKey, UWorld* World, IMovieScenePlayer& Player)
	{
		UAudioComponent* ExistingComponent = GetAudioComponent(FObjectKey(), SectionKey);
		if (!ExistingComponent)
		{
			USoundCue* TempPlaybackAudioCue = NewObject<USoundCue>();
			ExistingComponent = FAudioDevice::CreateComponent(TempPlaybackAudioCue, FAudioDevice::FCreateComponentParams(World));

			if (!ExistingComponent)
			{
				UE_LOG(LogMovieScene, Warning, TEXT("Failed to create audio component for master audio track (row %d)."), RowIndex);
				return nullptr;
			}

			Player.SavePreAnimatedState(*ExistingComponent, FMovieSceneAnimTypeID::Unique(), FDestroyDialogueSentencePreAnimatedToken::FProducer());

			ExistingComponent->SetFlags(RF_Transient);
			AudioComponentsByActorKey[FObjectKey()].Add(SectionKey, ExistingComponent);
		}

		return ExistingComponent;
	}

	void StopAllSounds()
	{
		for (TPair<FObjectKey, TMap<FObjectKey, TWeakObjectPtr<UAudioComponent>>>& Map : AudioComponentsByActorKey)
		{
			for (TPair<FObjectKey, TWeakObjectPtr<UAudioComponent>>& Pair : Map.Value)
			{
				if (UAudioComponent * AudioComponent = Pair.Value.Get())
				{
					AudioComponent->Stop();
				}
			}
		}
	}

	void StopSoundsOnSection(FObjectKey ObjectKey)
	{
		for (TPair<FObjectKey, TMap<FObjectKey, TWeakObjectPtr<UAudioComponent>>>& Pair : AudioComponentsByActorKey)
		{
			if (UAudioComponent * AudioComponent = Pair.Value.FindRef(ObjectKey).Get())
			{
				AudioComponent->Stop();
			}
		}
	}
};

struct FDialogueSentenceSectionExecutionToken : IMovieSceneExecutionToken
{
	FDialogueSentenceSectionExecutionToken(const UDialogueSentenceSection* InAudioSection)
		: DialogueSentenceSection(InAudioSection), SectionKey(InAudioSection)
	{}

	void Execute(const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand& Operand, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) override
	{
		FCachedDialogueSentenceTrackData& TrackData = PersistentData.GetOrAddTrackData<FCachedDialogueSentenceTrackData>();

		if ((Context.GetStatus() != EMovieScenePlayerStatus::Playing && Context.GetStatus() != EMovieScenePlayerStatus::Scrubbing) || Context.HasJumped() || Context.GetDirection() == EPlayDirection::Backwards)
		{
			// stopped, recording, etc
			TrackData.StopAllSounds();
		}

		// Master audio track
		else if (!Operand.ObjectBindingID.IsValid())
		{
			UObject* PlaybackContext = Player.GetPlaybackContext();

			UAudioComponent* AudioComponent = TrackData.GetAudioComponent(FObjectKey(), SectionKey);
			if (!AudioComponent)
			{
				// Initialize the sound
				AudioComponent = TrackData.AddMasterAudioComponentForRow(DialogueSentenceSection->GetRowIndex(), SectionKey, PlaybackContext ? PlaybackContext->GetWorld() : nullptr, Player);

//  				if (AudioComponent)
//  				{
//  					if (DialogueSentenceSection->GetOnQueueSubtitles().IsBound())
//  					{
//  						AudioComponent->OnQueueSubtitles = DialogueSentenceSection->GetOnQueueSubtitles();
//  					}
//  					if (DialogueSentenceSection->GetOnAudioFinished().IsBound())
//  					{
//  						AudioComponent->OnAudioFinished = DialogueSentenceSection->GetOnAudioFinished();
//  					}
//  					if (DialogueSentenceSection->GetOnAudioPlaybackPercent().IsBound())
//  					{
//  						AudioComponent->OnAudioPlaybackPercent = DialogueSentenceSection->GetOnAudioPlaybackPercent();
//  					}
//  				}
			}

			if (AudioComponent)
			{
				EnsureAudioIsPlaying(*AudioComponent, PersistentData, Context, false, Player);
			}
		}

		// Object binding audio track
		else
		{
			for (const TWeakObjectPtr<>& Object : Player.FindBoundObjects(Operand))
			{
				AActor* Actor = Cast<AActor>(Object.Get());
				if (!Actor)
				{
					continue;
				}

				UAudioComponent* AudioComponent = TrackData.GetAudioComponent(Actor, SectionKey);
				if (!AudioComponent)
				{
					// Initialize the sound
					AudioComponent = TrackData.AddAudioComponentForRow(DialogueSentenceSection->GetRowIndex(), SectionKey, *Actor, Player);

//  					if (AudioComponent)
//  					{
//  						if (DialogueSentenceSection->GetOnQueueSubtitles().IsBound())
//  						{
//  							AudioComponent->OnQueueSubtitles = DialogueSentenceSection->GetOnQueueSubtitles();
//  						}
//  						if (DialogueSentenceSection->GetOnAudioFinished().IsBound())
//  						{
//  							AudioComponent->OnAudioFinished = DialogueSentenceSection->GetOnAudioFinished();
//  						}
//  						if (DialogueSentenceSection->GetOnAudioPlaybackPercent().IsBound())
//  						{
//  							AudioComponent->OnAudioPlaybackPercent = DialogueSentenceSection->GetOnAudioPlaybackPercent();
//  						}
//  					}
				}

				if (AudioComponent)
				{
					EnsureAudioIsPlaying(*AudioComponent, PersistentData, Context, true, Player);
				}
			}
		}
	}

	void EnsureAudioIsPlaying(UAudioComponent& AudioComponent, FPersistentEvaluationData& PersistentData, const FMovieSceneContext& Context, bool bAllowSpatialization, IMovieScenePlayer& Player) const
	{
// 		Player.SavePreAnimatedState(AudioComponent, FStopDialogueSentencePreAnimatedToken::GetAnimTypeID(), FStopDialogueSentencePreAnimatedToken::FProducer());
// 
// 		bool bPlaySound = !AudioComponent.IsPlaying() || AudioComponent.Sound != DialogueSentenceSection->GetSound();
// 
// 		float AudioVolume = 1.f;
// 		DialogueSentenceSection->GetSoundVolumeChannel().Evaluate(Context.GetTime(), AudioVolume);
// 		AudioVolume = AudioVolume * DialogueSentenceSection->EvaluateEasing(Context.GetTime());
// 		if (AudioComponent.VolumeMultiplier != AudioVolume)
// 		{
// 			AudioComponent.SetVolumeMultiplier(AudioVolume);
// 		}
// 
// 		float PitchMultiplier = 1.f;
// 		DialogueSentenceSection->GetPitchMultiplierChannel().Evaluate(Context.GetTime(), PitchMultiplier);
// 		if (AudioComponent.PitchMultiplier != PitchMultiplier)
// 		{
// 			AudioComponent.SetPitchMultiplier(PitchMultiplier);
// 		}
// 
// 		if (bPlaySound)
// 		{
// 			AudioComponent.bAllowSpatialization = bAllowSpatialization;
// 
// 			if (DialogueSentenceSection->GetOverrideAttenuation())
// 			{
// 				AudioComponent.AttenuationSettings = DialogueSentenceSection->GetAttenuationSettings();
// 			}
// 
// 			AudioComponent.Stop();
// 			AudioComponent.SetSound(DialogueSentenceSection->GetSound());
// #if WITH_EDITOR
// 			UObject* PlaybackContext = Player.GetPlaybackContext();
// 			UWorld* World = PlaybackContext ? PlaybackContext->GetWorld() : nullptr;
// 			if (GIsEditor && World != nullptr && !World->IsPlayInEditor())
// 			{
// 				AudioComponent.bIsUISound = true;
// 				AudioComponent.bIsPreviewSound = true;
// 			}
// 			else
// #endif // WITH_EDITOR
// 			{
// 				AudioComponent.bIsUISound = false;
// 			}
// 
// 			float SectionStartTimeSeconds = (DialogueSentenceSection->HasStartFrame() ? DialogueSentenceSection->GetInclusiveStartFrame() : 0) / DialogueSentenceSection->GetTypedOuter<UMovieScene>()->GetTickResolution();
// 
// 			const float AudioTime = (Context.GetTime() / Context.GetFrameRate()) - SectionStartTimeSeconds + (float)Context.GetFrameRate().AsSeconds(DialogueSentenceSection->GetStartOffset());
// 			if (AudioTime >= 0.f && AudioComponent.Sound && AudioTime < AudioComponent.Sound->GetDuration())
// 			{
// 				AudioComponent.Play(AudioTime);
// 			}
// 
// 			if (Context.GetStatus() == EMovieScenePlayerStatus::Scrubbing)
// 			{
// 				// While scrubbing, play the sound for a short time and then cut it.
// 				AudioComponent.StopDelayed(AudioTrackConstants::ScrubDuration);
// 			}
// 		}
// 
// 		if (bAllowSpatialization)
// 		{
// 			if (FAudioDevice * AudioDevice = AudioComponent.GetAudioDevice())
// 			{
// 				DECLARE_CYCLE_STAT(TEXT("FAudioThreadTask.MovieSceneUpdateAudioTransform"), STAT_MovieSceneUpdateDialogueSentenceTransform, STATGROUP_TaskGraphTasks);
// 
// 				const FTransform ActorTransform = AudioComponent.GetOwner()->GetTransform();
// 				const uint64 ActorComponentID = AudioComponent.GetAudioComponentID();
// 				FAudioThread::RunCommandOnAudioThread([AudioDevice, ActorComponentID, ActorTransform]()
// 					{
// 						if (FActiveSound * ActiveSound = AudioDevice->FindActiveSound(ActorComponentID))
// 						{
// 							ActiveSound->bLocationDefined = true;
// 							ActiveSound->Transform = ActorTransform;
// 						}
// 					}, GET_STATID(STAT_MovieSceneUpdateDialogueSentenceTransform));
// 			}
// 		}
	}

	const UDialogueSentenceSection* DialogueSentenceSection;
	FObjectKey SectionKey;
};

void FDialogueSentenceSectionTemplate::Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const
{
	MOVIESCENE_DETAILED_SCOPE_CYCLE_COUNTER(MovieSceneEval_DialogueSentenceTrack_Evaluate)

	if (GEngine && GEngine->UseSound() && Context.GetStatus() != EMovieScenePlayerStatus::Jumping)
	{
		ExecutionTokens.Add(FDialogueSentenceSectionExecutionToken(AudioSection));
	}
}

void FDialogueSentenceSectionTemplate::TearDown(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const
{
	MOVIESCENE_DETAILED_SCOPE_CYCLE_COUNTER(MovieSceneEval_DialogueSentenceTrack_TearDown)

	if (GEngine && GEngine->UseSound())
	{
		FCachedDialogueSentenceTrackData& TrackData = PersistentData.GetOrAddTrackData<FCachedDialogueSentenceTrackData>();

		TrackData.StopSoundsOnSection(AudioSection);
	}
}

TOptional<TRange<FFrameNumber> > UDialogueSentenceSection::GetAutoSizeRange() const
{
	//FFrameRate FrameRate = GetTypedOuter<UMovieScene>()->GetTickResolution();

	return TRange<FFrameNumber>();
}

void UDialogueSentenceSection::TrimSection(FQualifiedFrameTime TrimTime, bool bTrimLeft)
{
	SetFlags(RF_Transactional);

	if (TryModify())
	{
		if (bTrimLeft)
		{
			StartFrameOffset = HasStartFrame() ? GetStartOffsetAtTrimTime(TrimTime, StartFrameOffset, GetInclusiveStartFrame()) : 0;
		}

		Super::TrimSection(TrimTime, bTrimLeft);
	}
}

UMovieSceneSection* UDialogueSentenceSection::SplitSection(FQualifiedFrameTime SplitTime)
{
	const FFrameNumber NewOffset = HasStartFrame() ? GetStartOffsetAtTrimTime(SplitTime, StartFrameOffset, GetInclusiveStartFrame()) : 0;

	UMovieSceneSection* NewSection = Super::SplitSection(SplitTime);
	if (NewSection != nullptr)
	{
		UDialogueSentenceSection* NewAudioSection = Cast<UDialogueSentenceSection>(NewSection);
		NewAudioSection->StartFrameOffset = NewOffset;
	}
	return NewSection;
}

TOptional<FFrameTime> UDialogueSentenceSection::GetOffsetTime() const
{
	return TOptional<FFrameTime>(StartFrameOffset);
}

FMovieSceneEvalTemplatePtr UDialogueSentenceSection::GenerateTemplate() const
{
	return FDialogueSentenceSectionTemplate(*this);
}

FFrameNumber UDialogueSentenceSection::GetStartOffsetAtTrimTime(FQualifiedFrameTime TrimTime, FFrameNumber StartOffset, FFrameNumber StartFrame)
{
	return StartOffset + TrimTime.Time.FrameNumber - StartFrame;
}
