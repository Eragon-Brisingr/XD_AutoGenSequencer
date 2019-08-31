// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovieSceneSection.h"
#include "MovieSceneEvalTemplate.h"
#include "MovieSceneFloatChannel.h"
#include "TwoTargetCameraTrackingSection.generated.h"

/**
 * 
 */
UCLASS()
class XD_AUTOGENSEQUENCER_API UTwoTargetCameraTrackingSection : public UMovieSceneSection
{
	GENERATED_BODY()
public:
	UTwoTargetCameraTrackingSection();

	//~ UMovieSceneSection interface
	TOptional<TRange<FFrameNumber> > GetAutoSizeRange() const override;
	void TrimSection(FQualifiedFrameTime TrimTime, bool bTrimLeft) override;
	UMovieSceneSection* SplitSection(FQualifiedFrameTime SplitTime) override;
	TOptional<FFrameTime> GetOffsetTime() const override;
	FMovieSceneEvalTemplatePtr GenerateTemplate() const override;

public:
	UPROPERTY(EditAnywhere, Category = CameraTracking)
	FMovieSceneObjectBindingID FrontTarget;

	UPROPERTY(EditAnywhere, Category = CameraTracking)
	FMovieSceneObjectBindingID BackTarget;

	UPROPERTY()
	FMovieSceneFloatChannel CameraYaw;
	UPROPERTY()
	FMovieSceneFloatChannel FrontTargetRate;
	UPROPERTY()
	FMovieSceneFloatChannel BackTargetRate;
};

USTRUCT()
struct FTwoTargetCameraTrackingSectionTemplate : public FMovieSceneEvalTemplate
{
	GENERATED_BODY()
public:
	FTwoTargetCameraTrackingSectionTemplate() = default;
	FTwoTargetCameraTrackingSectionTemplate(const UTwoTargetCameraTrackingSection& Section)
		:CameraTrackingSection(&Section)
	{}

	UPROPERTY()
	const UTwoTargetCameraTrackingSection* CameraTrackingSection;

private:
	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); }
	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override;
	virtual void SetupOverrides() override { EnableOverrides(RequiresTearDownFlag); }
	virtual void TearDown(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const override;
};

