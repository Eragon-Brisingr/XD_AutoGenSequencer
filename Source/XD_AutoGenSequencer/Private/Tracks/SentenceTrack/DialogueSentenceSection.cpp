// Fill out your copyright notice in the Description page of Project Settings.


#include "Tracks/SentenceTrack/DialogueSentenceSection.h"
#include <Engine/Engine.h>
#include <Components/AudioComponent.h>
#include "Sound/SoundCue.h"
#include "AudioDevice.h"
#include "XD_AutoGenSequencerUtils.h"
#include "Interface/DialogueInterface.h"
#include <MovieSceneCommonHelpers.h>
#include "IMovieScenePlayer.h"
#include "Data/DialogueSentence.h"

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

FCachedDialogueSentenceTrackData::FCachedDialogueSentenceTrackData()
{

}

void FCachedDialogueSentenceTrackData::StopAllSentences()
{
	for (TPair<TObjectKey<const UDialogueSentenceSection>, TWeakObjectPtr<UAudioComponent>>& Pair : AudioComponentBySectionKey)
	{
		if (UAudioComponent* AudioComponent = Pair.Value.Get())
		{
			StopSentence(AudioComponent);
#if WITH_EDITOR
			AudioComponent->bIsPreviewSound = false;
#endif
		}
	}
}

void FCachedDialogueSentenceTrackData::StopSentencesOnSection(TObjectKey<const UDialogueSentenceSection> ObjectKey)
{
	if (TWeakObjectPtr<UAudioComponent>* AudioComponentPtr = AudioComponentBySectionKey.Find(ObjectKey))
	{
		if (UAudioComponent* AudioComponent = AudioComponentPtr->Get())
		{
			StopSentence(AudioComponent);

#if WITH_EDITOR
			AudioComponent->bIsPreviewSound = false;
#endif
		}
		AudioComponentBySectionKey.Remove(ObjectKey);
	}
}

void FCachedDialogueSentenceTrackData::StopSentence(UAudioComponent* AudioComponent)
{
	AudioComponent->SetSound(nullptr);
	AActor* Actor = AudioComponent->GetOwner();
	if (Actor->Implements<UDialogueInterface>())
	{
		IDialogueInterface::EndSpeak(Actor);
	}
}

FDialogueSentenceSectionExecutionToken::FDialogueSentenceSectionExecutionToken(const UDialogueSentenceSection* InSentenceSection) 
	: SentenceSection(InSentenceSection), SectionKey(InSentenceSection)
{

}

void FDialogueSentenceSectionExecutionToken::Execute(const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand& Operand, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player)
{
	ExecutePlayAudio<FCachedDialogueSentenceTrackData>(Context, Operand, PersistentData, Player);
}

float FDialogueSentenceSectionExecutionToken::GetCurrentAudioTime(const FMovieSceneContext &Context) const
{
	float SectionStartTimeSeconds = (SentenceSection->HasStartFrame() ? SentenceSection->GetInclusiveStartFrame() : 0) / SentenceSection->GetTypedOuter<UMovieScene>()->GetTickResolution();
	const float AudioTime = (Context.GetTime() / Context.GetFrameRate()) - SectionStartTimeSeconds + (float)Context.GetFrameRate().AsSeconds(SentenceSection->StartFrameOffset);
	return AudioTime;
}

void FDialogueSentenceSectionExecutionToken::ExecutePlayAudioImpl(FCachedDialogueSentenceTrackData& TrackData, const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand& Operand, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player)
{
	MOVIESCENE_DETAILED_SCOPE_CYCLE_COUNTER(MovieSceneEval_DialogueSentenceTrack_TokenExecute);

	if ((Context.GetStatus() != EMovieScenePlayerStatus::Playing && Context.GetStatus() != EMovieScenePlayerStatus::Scrubbing) || Context.HasJumped() || Context.GetDirection() == EPlayDirection::Backwards)
	{
		// stopped, recording, etc
		TrackData.StopAllSentences();
	}
	else if (Operand.ObjectBindingID.IsValid() && SentenceSection->DialogueSentence)
	{
		USoundBase* SentenceSound = SentenceSection->DialogueSentence->SentenceWave;
		if (!SentenceSound)
		{
			return;
		}

		float AudioTime = GetCurrentAudioTime(Context);

		for (const TWeakObjectPtr<>& Object : Player.FindBoundObjects(Operand))
		{
			AActor* Actor = Cast<AActor>(Object.Get());
			if (!Actor)
			{
				continue;
			}

			TWeakObjectPtr<UAudioComponent>& AudioComponentPtr = TrackData.AudioComponentBySectionKey.FindOrAdd(SectionKey);
			if (!AudioComponentPtr.IsValid())
			{
				if (Actor->Implements<UDialogueInterface>())
				{
					AudioComponentPtr = IDialogueInterface::GetMouthComponent(Actor);
					IDialogueInterface::BeginSpeak(Actor, SentenceSection->DialogueSentence);
				}
			}

			if (UAudioComponent* AudioComponent = AudioComponentPtr.Get())
			{
				Player.SavePreAnimatedState(*AudioComponent, FStopDialogueSentencePreAnimatedToken::GetAnimTypeID(), FStopDialogueSentencePreAnimatedToken::FProducer());

				bool bPlaySound = !AudioComponent->IsPlaying() || AudioComponent->Sound != SentenceSound;

				if (bPlaySound && SentenceSound)
				{
#if WITH_EDITOR
					UObject* PlaybackContext = Player.GetPlaybackContext();
					UWorld* World = PlaybackContext ? PlaybackContext->GetWorld() : nullptr;
					if (GIsEditor && World != nullptr && !World->IsPlayInEditor())
					{
						AudioComponent->bIsPreviewSound = true;
					}
#endif // WITH_EDITOR

					if (!SentenceSound->IsPlayWhenSilent())
					{
						AutoGenSequencer_Error_Log("[%s] 的VirtualizationMode必须标记为PlayWhenSilent，否则超出声音区域后再进入声音会有问题，已自动设置，请保存音频文件", *SentenceSound->GetName());
						SentenceSound->Modify();
						SentenceSound->VirtualizationMode = EVirtualizationMode::PlayWhenSilent;
					}

					AudioComponent->Stop();
					AudioComponent->SetSound(SentenceSound);

					if (AudioTime >= 0.f && AudioComponent->Sound && AudioTime < AudioComponent->Sound->GetDuration())
					{
						AudioComponent->Play(AudioTime);
					}

					if (Context.GetStatus() == EMovieScenePlayerStatus::Scrubbing)
					{
						// While scrubbing, play the sound for a short time and then cut it.
						AudioComponent->StopDelayed(0.050f);
					}
				}

				//if (bAllowSpatialization)
				{
					if (FAudioDevice* AudioDevice = AudioComponent->GetAudioDevice())
					{
						DECLARE_CYCLE_STAT(TEXT("FAudioThreadTask.MovieSceneUpdateAudioTransform"), STAT_MovieSceneUpdateDialogueSentenceTransform, STATGROUP_TaskGraphTasks);

						const FTransform ActorTransform = AudioComponent->GetOwner()->GetTransform();
						const uint64 ActorComponentID = AudioComponent->GetAudioComponentID();
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
	TearDownAudio<FCachedDialogueSentenceTrackData>(PersistentData, Player);
}

TOptional<TRange<FFrameNumber>> UDialogueSentenceSection::GetAutoSizeRange() const
{
	if (DialogueSentence && DialogueSentence->SentenceWave)
	{
		float SoundDuration = DialogueSentence->GetDuration();

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

void UDialogueSentenceSection::TrimSection(FQualifiedFrameTime TrimTime, bool bTrimLeft, bool bDeleteKeys)
{
	SetFlags(RF_Transactional);

	if (TryModify())
	{
		if (bTrimLeft)
		{
			StartFrameOffset = HasStartFrame() ? GetStartOffsetAtTrimTime(TrimTime, StartFrameOffset, GetInclusiveStartFrame()) : 0;
		}

		Super::TrimSection(TrimTime, bTrimLeft, bDeleteKeys);
	}
}

UMovieSceneSection* UDialogueSentenceSection::SplitSection(FQualifiedFrameTime SplitTime, bool bDeleteKeys)
{
	const FFrameNumber NewOffset = HasStartFrame() ? GetStartOffsetAtTrimTime(SplitTime, StartFrameOffset, GetInclusiveStartFrame()) : 0;

	UMovieSceneSection* NewSection = Super::SplitSection(SplitTime, bDeleteKeys);
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
