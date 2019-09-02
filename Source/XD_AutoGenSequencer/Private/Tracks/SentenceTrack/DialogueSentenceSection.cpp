// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueSentenceSection.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "AudioDevice.h"
#include "XD_AutoGenSequencerUtils.h"
#include "Interface/DialogueInterface.h"
#include "MovieSceneCommonHelpers.h"
#include "IMovieScenePlayer.h"
#include "DialogueSentence.h"

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

struct FCachedDialogueSentenceTrackData : IPersistentEvaluationData
{
	TMap<FObjectKey, TWeakObjectPtr<UAudioComponent>> AudioComponentsByActorKey;
	TMap<FObjectKey, TWeakObjectPtr<USoundBase>> SoundsBySectionKey;

	FCachedDialogueSentenceTrackData()
	{

	}

	UAudioComponent* GetAudioComponent(FObjectKey SectionKey)
	{
		if (TWeakObjectPtr<UAudioComponent>* ExistingComponentPtr = AudioComponentsByActorKey.Find(SectionKey))
		{
			return ExistingComponentPtr->Get();
		}
		return nullptr;
	}

	/** Only to be called on the game thread */
	UAudioComponent* AddAudioComponentForRow(USoundBase* Sentence, FObjectKey SectionKey, AActor& PrincipalActor, IMovieScenePlayer& Player)
	{
		UAudioComponent* ExistingComponent = nullptr;
		if (PrincipalActor.Implements<UDialogueInterface>())
		{
			ExistingComponent = IDialogueInterface::GetMouthComponent(&PrincipalActor);
		}

		//测试用代码
		if (!ExistingComponent)
		{
			FAudioDevice::FCreateComponentParams Params(PrincipalActor.GetWorld(), &PrincipalActor);
			ExistingComponent = FAudioDevice::CreateComponent(Sentence, Params);

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
			//自动生成的需要销毁
			Player.SavePreAnimatedState(*ExistingComponent, FMovieSceneAnimTypeID::Unique(), FDestroyDialogueSentencePreAnimatedToken::FProducer());

			ExistingComponent->SetFlags(RF_Transient);
			ExistingComponent->AttachToComponent(PrincipalActor.GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		}

		AudioComponentsByActorKey.Add(SectionKey, ExistingComponent);

		return ExistingComponent;
	}

	void StopAllSentences()
	{
		for (TPair<FObjectKey, TWeakObjectPtr<UAudioComponent>>& Pair : AudioComponentsByActorKey)
		{
			if (UAudioComponent* AudioComponent = Pair.Value.Get())
			{
				AudioComponent->Stop();
#if WITH_EDITOR
				AudioComponent->bIsPreviewSound = false;
#endif
			}
		}
	}

	void StopSentencesOnSection(FObjectKey ObjectKey)
	{
		if (TWeakObjectPtr<UAudioComponent>* AudioComponentPtr = AudioComponentsByActorKey.Find(ObjectKey))
		{
			if (UAudioComponent* AudioComponent = AudioComponentPtr->Get())
			{
				AudioComponent->Stop();
#if WITH_EDITOR
				AudioComponent->bIsPreviewSound = false;
#endif
			}
			AudioComponentsByActorKey.Remove(ObjectKey);
		}
		SoundsBySectionKey.Remove(ObjectKey);
	}
};

FDialogueSentenceSectionExecutionToken::FDialogueSentenceSectionExecutionToken(const UDialogueSentenceSection* InSentenceSection) 
	: SentenceSection(InSentenceSection), SectionKey(InSentenceSection)
{

}

void FDialogueSentenceSectionExecutionToken::Execute(const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand& Operand, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player)
{
	MOVIESCENE_DETAILED_SCOPE_CYCLE_COUNTER(MovieSceneEval_DialogueSentenceTrack_TokenExecute);

	FCachedDialogueSentenceTrackData& TrackData = PersistentData.GetOrAddTrackData<FCachedDialogueSentenceTrackData>();

	if ((Context.GetStatus() != EMovieScenePlayerStatus::Playing && Context.GetStatus() != EMovieScenePlayerStatus::Scrubbing) || Context.HasJumped() || Context.GetDirection() == EPlayDirection::Backwards)
	{
		// stopped, recording, etc
		TrackData.StopAllSentences();
	}
	else if (Operand.ObjectBindingID.IsValid())
	{
		for (const TWeakObjectPtr<>& Object : Player.FindBoundObjects(Operand))
		{
			AActor* Actor = Cast<AActor>(Object.Get());
			if (!Actor)
			{
				continue;
			}

			TWeakObjectPtr<USoundBase>& SentenceSoundPtr = TrackData.SoundsBySectionKey.FindOrAdd(SectionKey);
			if (!SentenceSoundPtr.IsValid())
			{
				SentenceSoundPtr = SentenceSection->DialogueSentence->SentenceWave;
				if (!SentenceSoundPtr.IsValid())
				{
					continue;
				}
			}
			USoundBase* SentenceSound = SentenceSoundPtr.Get();

			UAudioComponent* AudioComponent = TrackData.GetAudioComponent(SectionKey);
			if (!AudioComponent)
			{
				AudioComponent = TrackData.AddAudioComponentForRow(SentenceSound, SectionKey, *Actor, Player);
			}

			if (AudioComponent)
			{
				EnsureSentenceIsPlaying(SentenceSound, *AudioComponent, PersistentData, Context, true, Player);
			}
		}
	}
}

void FDialogueSentenceSectionExecutionToken::EnsureSentenceIsPlaying(USoundBase* SentenceSound, UAudioComponent& AudioComponent, FPersistentEvaluationData& PersistentData, const FMovieSceneContext& Context, bool bAllowSpatialization, IMovieScenePlayer& Player) const
{
	Player.SavePreAnimatedState(AudioComponent, FStopDialogueSentencePreAnimatedToken::GetAnimTypeID(), FStopDialogueSentencePreAnimatedToken::FProducer());

	bool bPlaySound = !AudioComponent.IsPlaying() || AudioComponent.Sound != SentenceSound;

	if (bPlaySound && SentenceSound)
	{
#if WITH_EDITOR
		UObject* PlaybackContext = Player.GetPlaybackContext();
		UWorld* World = PlaybackContext ? PlaybackContext->GetWorld() : nullptr;
		if (GIsEditor && World != nullptr && !World->IsPlayInEditor())
		{
			AudioComponent.bIsPreviewSound = true;
		}
#endif // WITH_EDITOR

		if (!SentenceSound->IsPlayWhenSilent())
		{
			AutoGenSequencer_Error_Log("[%s] 的VirtualizationMode必须标记为PlayWhenSilent，否则超出声音区域后再进入声音会有问题，已自动设置，请保存音频文件", *SentenceSound->GetName());
			SentenceSound->Modify();
			SentenceSound->VirtualizationMode = EVirtualizationMode::PlayWhenSilent;
		}

		AudioComponent.Stop();
		AudioComponent.SetSound(SentenceSound);

		float SectionStartTimeSeconds = (SentenceSection->HasStartFrame() ? SentenceSection->GetInclusiveStartFrame() : 0) / SentenceSection->GetTypedOuter<UMovieScene>()->GetTickResolution();

		const float AudioTime = (Context.GetTime() / Context.GetFrameRate()) - SectionStartTimeSeconds + (float)Context.GetFrameRate().AsSeconds(SentenceSection->StartFrameOffset);
		if (AudioTime >= 0.f && AudioComponent.Sound && AudioTime < AudioComponent.Sound->GetDuration())
		{
			AudioComponent.Play(AudioTime);
		}

		if (Context.GetStatus() == EMovieScenePlayerStatus::Scrubbing)
		{
			// While scrubbing, play the sound for a short time and then cut it.
			AudioComponent.StopDelayed(0.050f);
		}
	}

	if (bAllowSpatialization)
	{
		if (FAudioDevice* AudioDevice = AudioComponent.GetAudioDevice())
		{
			DECLARE_CYCLE_STAT(TEXT("FAudioThreadTask.MovieSceneUpdateAudioTransform"), STAT_MovieSceneUpdateDialogueSentenceTransform, STATGROUP_TaskGraphTasks);

			const FTransform ActorTransform = AudioComponent.GetOwner()->GetTransform();
			const uint64 ActorComponentID = AudioComponent.GetAudioComponentID();
			FAudioThread::RunCommandOnAudioThread([AudioDevice, ActorComponentID, ActorTransform]()
				{
					if (FActiveSound * ActiveSound = AudioDevice->FindActiveSound(ActorComponentID))
					{
						ActiveSound->bLocationDefined = true;
						ActiveSound->Transform = ActorTransform;
					}
				}, GET_STATID(STAT_MovieSceneUpdateDialogueSentenceTransform));
		}
	}
}

void FDialogueSentenceSectionTemplate::Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const
{
	MOVIESCENE_DETAILED_SCOPE_CYCLE_COUNTER(MovieSceneEval_DialogueSentenceTrack_Evaluate)

	if (GEngine && GEngine->UseSound() && Context.GetStatus() != EMovieScenePlayerStatus::Jumping)
	{
		ExecutionTokens.Add(FDialogueSentenceSectionExecutionToken(SentenceSection));
	}
}

void FDialogueSentenceSectionTemplate::TearDown(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const
{
	MOVIESCENE_DETAILED_SCOPE_CYCLE_COUNTER(MovieSceneEval_DialogueSentenceTrack_TearDown)

	if (GEngine && GEngine->UseSound())
	{
		FCachedDialogueSentenceTrackData& TrackData = PersistentData.GetOrAddTrackData<FCachedDialogueSentenceTrackData>();
		TrackData.StopSentencesOnSection(SentenceSection);
	}
}

TOptional<TRange<FFrameNumber>> UDialogueSentenceSection::GetAutoSizeRange() const
{
	USoundBase* SentenceSound = GetDefualtSentenceSound();
	if (SentenceSound)
	{
		float SoundDuration = MovieSceneHelpers::GetSoundDuration(SentenceSound);

		FFrameRate FrameRate = GetTypedOuter<UMovieScene>()->GetTickResolution();

		// determine initial duration
		// @todo Once we have infinite sections, we can remove this
		// @todo ^^ Why? Infinte sections would mean there's no starting time?
		FFrameTime DurationToUse = 1.f * FrameRate; // if all else fails, use 1 second duration

		if (SoundDuration != INDEFINITELY_LOOPING_DURATION)
		{
			DurationToUse = SoundDuration * FrameRate;
		}

		return TRange<FFrameNumber>(GetInclusiveStartFrame(), GetInclusiveStartFrame() + DurationToUse.FrameNumber);
	}
	return TOptional<TRange<FFrameNumber>>();
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

USoundBase* UDialogueSentenceSection::GetDefualtSentenceSound() const
{
	return DialogueSentence->SentenceWave;
}

FFrameNumber UDialogueSentenceSection::GetStartOffsetAtTrimTime(FQualifiedFrameTime TrimTime, FFrameNumber StartOffset, FFrameNumber StartFrame)
{
	return StartOffset + TrimTime.Time.FrameNumber - StartFrame;
}
