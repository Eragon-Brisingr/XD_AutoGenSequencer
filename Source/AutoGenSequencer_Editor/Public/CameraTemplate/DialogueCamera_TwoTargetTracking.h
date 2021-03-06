﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CameraTemplate/AutoGenDialogueCameraTemplate.h"
#include "DialogueCamera_TwoTargetTracking.generated.h"

class UTextRenderComponent;
class ACharacter;

/**
 * 
 */
UCLASS(meta = (DisplayName = "双目标镜头"))
class AUTOGENSEQUENCER_EDITOR_API ADialogueCamera_TwoTargetTracking : public AAutoGenDialogueCameraTemplate
{
	GENERATED_BODY()

public:
	ADialogueCamera_TwoTargetTracking();

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void UpdateCameraTransform() override;
	
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
public:
	UPROPERTY()
	UChildActorComponent* FrontCharacterComponent;
	UPROPERTY()
	UChildActorComponent* BackCharacterComponent;

	UPROPERTY(EditAnywhere, Category = "预览配置", meta = (DisplayName = "前景角色类型"))
	TSubclassOf<ACharacter> FrontCharacterType;
	UPROPERTY(EditAnywhere, Category = "预览配置", meta = (DisplayName = "背景角色类型"))
	TSubclassOf<ACharacter> BackCharacterType;
public:
	TOptional<FCameraWeightsData> EvaluateCameraTemplate(ACharacter* LookTarget, const TArray<ACharacter*>& Others, const TMap<ACharacter*, FGenDialogueCharacterData>& DialogueCharacterDataMap, float DialogueProgress) const override;
	void GenerateCameraTrackData(ACharacter* LookTarget, const TArray<ACharacter*>& Others, UMovieScene& MovieScene, FGuid CineCameraComponentGuid, const TMap<ACharacter*, FGenDialogueCharacterData>& DialogueCharacterDataMap, const TArray<FDialogueCameraCutData>& DialogueCameraCutDatas) const override;
};
