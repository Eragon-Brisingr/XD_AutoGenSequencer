// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovieSceneSection.h"
#include <Evaluation/MovieSceneEvalTemplate.h>
#include <Channels/MovieSceneFloatChannel.h>
#include "Utils/SequenceSectionPreviewInfo.h"
#include "TwoTargetCameraTrackingSection.generated.h"

/**
 * 
 */
UCLASS()
class AUTOGENSEQUENCER_API UTwoTargetCameraTrackingSection : public UMovieSceneSection, public ISequenceSectionPreviewInfo
{
	GENERATED_BODY()
public:
	UTwoTargetCameraTrackingSection();

	//~ UMovieSceneSection interface
	TOptional<TRange<FFrameNumber>> GetAutoSizeRange() const override;
	void TrimSection(FQualifiedFrameTime TrimTime, bool bTrimLeft, bool bDeleteKeys) override;
	UMovieSceneSection* SplitSection(FQualifiedFrameTime SplitTime, bool bDeleteKeys) override;
	TOptional<FFrameTime> GetOffsetTime() const override;
	FMovieSceneEvalTemplatePtr GenerateTemplate() const override;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "调试")
	FLinearColor DebugColor = FLinearColor::Red;
	// ISequenceSectionPreviewInfo
	void DrawSectionSelectedPreviewInfo(IMovieScenePlayer* Player, const FFrameNumber& FramePosition, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) const override;
	void DrawSectionExecutePreviewInfo(IMovieScenePlayer* Player, const FFrameNumber& FramePosition, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) const override;
#endif
public:
	UPROPERTY(EditAnywhere, Category = CameraTracking)
	FMovieSceneObjectBindingID FrontTarget;
	UPROPERTY(EditAnywhere, Category = CameraTracking)
	FVector FrontOffset;
	UPROPERTY(EditAnywhere, Category = CameraTracking)
	FMovieSceneObjectBindingID BackTarget;
	UPROPERTY(EditAnywhere, Category = CameraTracking)
	FName BackTargetTrackingSocketName = TEXT("head");
	UPROPERTY(EditAnywhere, Category = CameraTracking)
	float BackTargetVolumnRadius = 10.f;
	UPROPERTY(EditAnywhere, Category = CameraTracking)
	float BackTolerance = 0.05f;
	UPROPERTY(EditAnywhere, Category = CameraTracking)
	FVector BackOffset;
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = CameraTracking)
	UClass* CreateFrom;
#endif
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

	void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override;
	void SetupOverrides() override { EnableOverrides(RequiresTearDownFlag); }
	void TearDown(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const override;

private:
};

