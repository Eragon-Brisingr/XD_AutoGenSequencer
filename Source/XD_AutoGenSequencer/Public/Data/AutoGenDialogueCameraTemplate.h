// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AutoGenDialogueCameraTemplate.generated.h"

class UChildActorComponent;
class ADialogueStandPositionTemplate;
class UCineCameraComponent;

UCLASS(Transient, abstract, Blueprintable, hidedropdown, hidecategories = (Input, Movement, Collision, Rendering, Replication, Actor, LOD, Cooking))
class XD_AUTOGENSEQUENCER_API AAutoGenDialogueCameraTemplate : public AActor
{
	GENERATED_BODY()

public:
	AAutoGenDialogueCameraTemplate();
#if WITH_EDITORONLY_DATA
	void OnConstruction(const FTransform& Transform) override;

	void PreEditChange(UProperty* PropertyThatWillChange) override;
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	UPROPERTY()
	UChildActorComponent* StandTemplatePreview;

	UPROPERTY(VisibleAnywhere, Transient)
	UCineCameraComponent* CineCameraComponent;

	UPROPERTY()
	UChildActorComponent* CineCamera;

	UPROPERTY(EditAnywhere, Category = "镜头模板", meta = (DisplayName = "站位模板"))
	TSubclassOf<ADialogueStandPositionTemplate> StandTemplate;

	UPROPERTY(EditAnywhere, Category = "镜头模板", meta = (ClampMin = "0", ClampMax = "0.499999"))
	float BackTargetRate;
	UPROPERTY(EditAnywhere, Category = "镜头模板", meta = (ClampMin = "0", ClampMax = "0.499999"))
	float FrontTargetRate;
	UPROPERTY(EditAnywhere, Category = "镜头模板", meta = (ClampMin = "0", ClampMax = "180"))
	float CameraYawAngle;
#endif
};
