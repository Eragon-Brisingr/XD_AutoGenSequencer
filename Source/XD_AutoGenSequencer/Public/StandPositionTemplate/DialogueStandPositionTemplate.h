// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "DialogueStandPositionTemplate.generated.h"

class ACharacter;
class UChildActorComponent;

USTRUCT()
struct FDialogueStandPosition
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, meta = (MakeEditWidget = true))
	FTransform StandPosition;

	UPROPERTY(EditAnywhere)
	FName StandName;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ACharacter> PreviewCharacter;

	UPROPERTY(Transient)
	UChildActorComponent* PreviewCharacterInstance;
};

UCLASS(Transient, abstract, Blueprintable, hidedropdown, hidecategories = (Input, Movement, Collision, Rendering, Replication, Actor, LOD, Cooking), showCategories = ("Utilities|Transformation"))
class XD_AUTOGENSEQUENCER_API ADialogueStandPositionTemplate : public AInfo
{
	GENERATED_BODY()
public:
#if WITH_EDITORONLY_DATA
	ADialogueStandPositionTemplate();

	UPROPERTY(EditAnywhere, Category = "站位模板")
	TSubclassOf<ACharacter> PreviewCharacter;

	UPROPERTY(EditAnywhere, Category = "站位模板")
	TArray<FDialogueStandPosition> StandPositions;

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	void OnConstruction(const FTransform& Transform) override;

	UChildActorComponent* CreateChildActorComponent();

	void CreateAllTemplatePreviewCharacter();

	uint8 bSpawnedPreviewCharacter : 1;

	void ClearInvalidPreviewCharacter();

	//应用站位坐标
	UFUNCTION(CallInEditor, meta = (DisplayName = "应用站位坐标"), Category = "站位模板")
	void ApplyStandPositionsToDefault();

	UPROPERTY(Transient)
	TSet<UChildActorComponent*> PreviewCharacters;

	DECLARE_DELEGATE(FOnInstanceChanged);
	FOnInstanceChanged OnInstanceChanged;
#endif
};
