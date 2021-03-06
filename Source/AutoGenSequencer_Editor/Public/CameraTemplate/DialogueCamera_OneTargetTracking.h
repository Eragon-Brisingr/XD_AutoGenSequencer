// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CameraTemplate/AutoGenDialogueCameraTemplate.h"
#include "DialogueCamera_OneTargetTracking.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "单目标镜头"))
class AUTOGENSEQUENCER_EDITOR_API ADialogueCamera_OneTargetTracking : public AAutoGenDialogueCameraTemplate
{
	GENERATED_BODY()
public:
    ADialogueCamera_OneTargetTracking();

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void UpdateCameraTransform() override;

    UPROPERTY(EditAnywhere, Category = "镜头模板", meta = (MakeEditWidget = true))
    FVector CameraLocation = FVector(100.f, 100.f, 170.f);
    UPROPERTY(EditAnywhere, Category = "镜头模板")
	FName SocketName = TEXT("head");
    UPROPERTY(EditAnywhere, Category = "镜头模板")
    FVector TargetOffset = FVector(10.f, 0.f, 0.f);

    UPROPERTY(VisibleAnywhere, Category = "镜头模板", AdvancedDisplay)
    FVector CameraRelativeLocation;
    UPROPERTY(VisibleAnywhere, Category = "镜头模板", AdvancedDisplay)
    FRotator CameraRelativeRotation;
    
    UPROPERTY()
	UChildActorComponent* CameraTargetCharacter;
    UPROPERTY(EditAnywhere, Category = "预览配置", meta = (DisplayName = "注视角色类型"))
	TSubclassOf<ACharacter> TargetCharacterType;

    TOptional<FCameraWeightsData> EvaluateCameraTemplate(ACharacter* LookTarget, const TArray<ACharacter*>& Others, const TMap<ACharacter*, FGenDialogueCharacterData>& DialogueCharacterDataMap, float DialogueProgress) const override;
    void GenerateCameraTrackData(ACharacter* LookTarget, const TArray<ACharacter*>& Others, UMovieScene& MovieScene, FGuid CineCameraComponentGuid, const TMap<ACharacter*, FGenDialogueCharacterData>& DialogueCharacterDataMap, const TArray<FDialogueCameraCutData>& DialogueCameraCutDatas) const override;
};
