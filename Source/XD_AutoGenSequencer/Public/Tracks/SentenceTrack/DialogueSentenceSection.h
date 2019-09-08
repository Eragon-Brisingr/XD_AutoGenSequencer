﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovieSceneSection.h"
#include "MovieSceneEvalTemplate.h"
#include "DialogueSentenceSection.generated.h"

class USoundBase;
class UDialogueSentence;
class UAudioComponent;

/**
 * 
 */
UCLASS()
class XD_AUTOGENSEQUENCER_API UDialogueSentenceSection : public UMovieSceneSection
{
	GENERATED_BODY()
public:
	//~ UMovieSceneSection interface
	TOptional<TRange<FFrameNumber> > GetAutoSizeRange() const override;
	void TrimSection(FQualifiedFrameTime TrimTime, bool bTrimLeft) override;
	UMovieSceneSection* SplitSection(FQualifiedFrameTime SplitTime) override;
	TOptional<FFrameTime> GetOffsetTime() const override;
	FMovieSceneEvalTemplatePtr GenerateTemplate() const override;

public:
	/** The offset into the beginning of the audio clip */
	UPROPERTY(EditAnywhere, Category = Dialogue)
	FFrameNumber StartFrameOffset;

	UPROPERTY(EditAnywhere, Category = Dialogue)
	UDialogueSentence* DialogueSentence;

	UPROPERTY(EditAnywhere, Category = Dialogue)
	TArray<FMovieSceneObjectBindingID> Targets;
private:
	static FFrameNumber GetStartOffsetAtTrimTime(FQualifiedFrameTime TrimTime, FFrameNumber StartOffset, FFrameNumber StartFrame);
};

USTRUCT()
struct XD_AUTOGENSEQUENCER_API FDialogueSentenceSectionTemplate : public FMovieSceneEvalTemplate
{
	GENERATED_BODY()
public:
	FDialogueSentenceSectionTemplate() = default;
	FDialogueSentenceSectionTemplate(const UDialogueSentenceSection& Section)
		:SentenceSection(&Section)
	{}

	UPROPERTY()
	const UDialogueSentenceSection* SentenceSection;

private:
	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); }
	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override;
	virtual void SetupOverrides() override { EnableOverrides(RequiresTearDownFlag); }
	virtual void TearDown(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const override;

protected:
	template<typename DataType>
	void TearDownAudio(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const
	{
		MOVIESCENE_DETAILED_SCOPE_CYCLE_COUNTER(MovieSceneEval_DialogueSentenceTrack_TearDown)

		static_assert(TIsDerivedFrom<DataType, FCachedDialogueSentenceTrackData>::IsDerived, "Must Derived From FCachedDialogueSentenceTrackData");
		if (GEngine && GEngine->UseSound())
		{
			FCachedDialogueSentenceTrackData& TrackData = PersistentData.GetOrAddTrackData<DataType>();
			TrackData.StopSentencesOnSection(SentenceSection);
		}
	}
};

struct XD_AUTOGENSEQUENCER_API FCachedDialogueSentenceTrackData : IPersistentEvaluationData
{
	TMap<TObjectKey<const UDialogueSentenceSection>, TWeakObjectPtr<UAudioComponent>> AudioComponentBySectionKey;

	FCachedDialogueSentenceTrackData();

	void StopAllSentences();

	void StopSentencesOnSection(TObjectKey<const UDialogueSentenceSection> ObjectKey);

	void StopSentence(UAudioComponent* AudioComponent);

};

struct XD_AUTOGENSEQUENCER_API FDialogueSentenceSectionExecutionToken : IMovieSceneExecutionToken
{
	FDialogueSentenceSectionExecutionToken(const UDialogueSentenceSection* InSentenceSection);

	void Execute(const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand& Operand, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) override;

	const UDialogueSentenceSection* SentenceSection;
	TObjectKey<const UDialogueSentenceSection> SectionKey;
	TMap<AActor*, USoundBase*> SentenceSoundMap;

protected:
	float GetCurrentAudioTime(const FMovieSceneContext &Context) const;

	template<typename DataType>
	void ExecutePlayAudio(const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand& Operand, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player)
	{
		static_assert(TIsDerivedFrom<DataType, FCachedDialogueSentenceTrackData>::IsDerived, "Must Derived From FCachedDialogueSentenceTrackData");
		FCachedDialogueSentenceTrackData& TrackData = PersistentData.GetOrAddTrackData<DataType>();
		ExecutePlayAudioImpl(TrackData, Context, Operand, PersistentData, Player);
	}
private:
	void ExecutePlayAudioImpl(FCachedDialogueSentenceTrackData& TrackData, const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand& Operand, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player);
};
