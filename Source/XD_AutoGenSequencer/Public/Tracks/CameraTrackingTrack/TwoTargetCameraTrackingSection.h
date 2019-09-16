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

	// TODO：只会在解算开始执行一次
	UPROPERTY(EditAnywhere, Category = CameraTracking)
	uint8 bOnlyInitializeEvaluate : 1;

	// TODO：保持初始化时传入目标的位置
	UPROPERTY(EditAnywhere, Category = CameraTracking)
	uint8 bKeepInitializeTargetLocation : 1;

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
	UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); }

	// TODO：确认这个的执行时机，貌似不是Section触发时执行
	//void Initialize(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const override;

	void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override;
	void SetupOverrides() override { EnableOverrides(RequiresTearDownFlag/* | EOverrideMask::RequiresInitializeFlag*/); }
	void TearDown(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const override;

private:
};

