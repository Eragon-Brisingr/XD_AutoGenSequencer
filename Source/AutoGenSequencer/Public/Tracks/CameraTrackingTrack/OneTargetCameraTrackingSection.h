// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovieSceneSection.h"
#include "Evaluation/MovieSceneEvalTemplate.h"
#include "OneTargetCameraTrackingSection.generated.h"

/**
 * 
 */
UCLASS()
class AUTOGENSEQUENCER_API UOneTargetCameraTrackingSection : public UMovieSceneSection
{
	GENERATED_BODY()
public:
    FMovieSceneEvalTemplatePtr GenerateTemplate() const override;

public:
	UPROPERTY(EditAnywhere, Category = CameraTracking)
	FMovieSceneObjectBindingID Target;
	UPROPERTY(EditAnywhere, Category = CameraTracking)
	FName SocketName = TEXT("head");
	UPROPERTY(EditAnywhere, Category = CameraTracking)
	FVector Offset;
	UPROPERTY(EditAnywhere, Category = CameraTracking)
    FVector RelativeLocation;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = CameraTracking)
	UClass* CreateFrom;
#endif
};

USTRUCT()
struct FOneTargetCameraTrackingSectionTemplate : public FMovieSceneEvalTemplate
{
	GENERATED_BODY()
public:
	FOneTargetCameraTrackingSectionTemplate() = default;
	FOneTargetCameraTrackingSectionTemplate(const UOneTargetCameraTrackingSection& Section)
		:CameraTrackingSection(&Section)
	{}

	UPROPERTY()
	const UOneTargetCameraTrackingSection* CameraTrackingSection;

private:
	UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); }

	void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override;
	void SetupOverrides() override { EnableOverrides(RequiresTearDownFlag); }
	void TearDown(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const override;

private:
};
