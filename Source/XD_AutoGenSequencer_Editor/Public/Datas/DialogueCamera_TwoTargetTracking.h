﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Datas/AutoGenDialogueCameraTemplate.h"
#include "DialogueCamera_TwoTargetTracking.generated.h"

class UTextRenderComponent;

/**
 * 
 */
UCLASS(meta = (DisplayName = "双目标追踪镜头模板"))
class XD_AUTOGENSEQUENCER_EDITOR_API ADialogueCamera_TwoTargetTracking : public AAutoGenDialogueCameraTemplate
{
	GENERATED_BODY()

public:
	ADialogueCamera_TwoTargetTracking();

	void OnConstruction(const FTransform& Transform) override;
	
	UPROPERTY(EditAnywhere, Category = "镜头模板", meta = (DisplayName = "前景对象占比", ClampMin = "-1", ClampMax = "0.499999"))
	float FrontTargetRate = 0.1f;
	UPROPERTY(EditAnywhere, Category = "镜头模板", meta = (DisplayName = "前景焦点偏移"))
	FVector FrontOffset;
	UPROPERTY(EditAnywhere, Category = "镜头模板", meta = (DisplayName = "背景对象占比", ClampMin = "-1", ClampMax = "0.499999"))
	float BackTargetRate = 0.3f;
	UPROPERTY(EditAnywhere, Category = "镜头模板", meta = (DisplayName = "背景焦点偏移"))
	FVector BackOffset;
	UPROPERTY(EditAnywhere, Category = "镜头模板", meta = (DisplayName = "镜头偏航角", ClampMin = "0.0001", ClampMax = "90"))
	float CameraYawAngle = 20.f;
	
	UPROPERTY(EditAnywhere, Category = "镜头模板", meta = (DisplayName = "预览前景对象编号"))
	uint8 PreviewFrontTargetIdx = 0;
	UPROPERTY(EditAnywhere, Category = "镜头模板", meta = (DisplayName = "预览背景对象编号"))
	uint8 PreviewBackTargetIdx = 1;

public:
	UPROPERTY(Transient)
	UTextRenderComponent* PreviewFrontHint;
	UPROPERTY(VisibleAnywhere)
	UChildActorComponent* FrontCharacterComponent;
	UPROPERTY(Transient)
	UTextRenderComponent* PreviewBackHint;
	UPROPERTY(VisibleAnywhere)
	UChildActorComponent* BackCharacterComponent;
public:
	TOptional<FCameraWeightsData> EvaluateCameraTemplate(ACharacter* LookTarget, const TArray<ACharacter*>& Others, const TMap<ACharacter*, FGenDialogueCharacterData>& DialogueCharacterDataMap, float DialogueProgress) const override;
	void GenerateCameraTrackData(ACharacter* LookTarget, const TArray<ACharacter*>& Others, UMovieScene& MovieScene, FGuid CineCameraComponentGuid, const TMap<ACharacter*, FGenDialogueCharacterData>& DialogueCharacterDataMap, const TArray<FDialogueCameraCutData>& DialogueCameraCutDatas) const override;
};
