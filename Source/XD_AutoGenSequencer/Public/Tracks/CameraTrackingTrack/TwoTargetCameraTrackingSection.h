// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovieSceneSection.h"
#include "MovieSceneEvalTemplate.h"
#include "MovieSceneFloatChannel.h"
#include "XD_SequenceSectionPreviewInfo.h"
#include "TwoTargetCameraTrackingSection.generated.h"

/**
 * 
 */
UCLASS()
class XD_AUTOGENSEQUENCER_API UTwoTargetCameraTrackingSection : public UMovieSceneSection, public IXD_SequenceSectionPreviewInfo
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

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "调试")
	FLinearColor DebugColor = FLinearColor::Red;
	// IXD_SequenceSectionPreviewInfo
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

	// TODO：确认这个的执行时机，貌似不是Section触发时执行
	//void Initialize(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const override;

	void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override;
	void SetupOverrides() override { EnableOverrides(RequiresTearDownFlag/* | EOverrideMask::RequiresInitializeFlag*/); }
	void TearDown(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const override;

private:
};

