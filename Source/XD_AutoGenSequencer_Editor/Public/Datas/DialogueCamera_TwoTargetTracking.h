// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Datas/AutoGenDialogueCameraTemplate.h"
#include "DialogueCamera_TwoTargetTracking.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "双目标追踪镜头模板"))
class XD_AUTOGENSEQUENCER_EDITOR_API ADialogueCamera_TwoTargetTracking : public AAutoGenDialogueCameraTemplate
{
	GENERATED_BODY()

public:
	void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(EditAnywhere, Category = "镜头模板", meta = (ClampMin = "-1", ClampMax = "0.499999"))
	float BackTargetRate;
	UPROPERTY(EditAnywhere, Category = "镜头模板", meta = (ClampMin = "-1", ClampMax = "0.499999"))
	float FrontTargetRate;
	UPROPERTY(EditAnywhere, Category = "镜头模板", meta = (ClampMin = "0", ClampMax = "180"))
	float CameraYawAngle;

	FCameraWeightsData EvaluateCameraTemplate(ACharacter* Speaker, const TArray<ACharacter*>& Targets, float DialogueProgress) const override;

	void GenerateCameraTrackData(ACharacter* Speaker, const TArray<ACharacter*>& Targets, UMovieScene& MovieScene, FGuid CineCameraComponentGuid, const TMap<ACharacter*, FMovieSceneObjectBindingID>& InstanceBindingIdMap, const TMap<ACharacter*, int32>& InstanceIdxMap) const override;
};
