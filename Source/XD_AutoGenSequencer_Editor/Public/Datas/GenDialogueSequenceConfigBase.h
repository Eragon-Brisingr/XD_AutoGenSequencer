// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SoftObjectPtr.h"
#include "SubclassOf.h"
#include "GenDialogueSequenceConfigBase.generated.h"

class ISequencer;
class ACharacter;
class UPreviewDialogueSoundSequence;
class UAutoGenDialogueSequence;
class ADialogueStandPositionTemplate;
class UAutoGenDialogueAnimSet;

/**
 *
 */
USTRUCT()
struct XD_AUTOGENSEQUENCER_EDITOR_API FDialogueCharacterData
{
	GENERATED_BODY()
public:
	FDialogueCharacterData();

	UPROPERTY(EditAnywhere)
	FName NameOverride;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<ACharacter> InstanceOverride;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ACharacter> TypeOverride;
	
	UPROPERTY(EditAnywhere)
 	FTransform PositionOverride;

	UPROPERTY(EditAnywhere)
	FName TalkAnimSlotName;

	UPROPERTY(EditAnywhere)
	UAutoGenDialogueAnimSet* DialogueAnimSet;
};

USTRUCT()
struct XD_AUTOGENSEQUENCER_EDITOR_API FDialogueStationInstance
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "站位模板"))
	TSubclassOf<ADialogueStandPositionTemplate> DialogueStationTemplate;

	UPROPERTY(EditAnywhere, EditFixedSize = true, meta = (DisplayName = "对话角色"))
	TArray<FDialogueCharacterData> DialogueCharacterDatas;
	
#if WITH_EDITORONLY_DATA
	void SyncInstanceData(const ADialogueStandPositionTemplate* Instance);
	bool IsValid() const;
	TArray<FName> GetCharacterNames() const;

	TArray<TSharedPtr<FString>> DialogueNameList;
	TArray<TSharedPtr<FString>>& GetDialogueNameList();
	void ReinitDialogueNameList();
#endif
};

UCLASS(abstract)
class XD_AUTOGENSEQUENCER_EDITOR_API UGenDialogueSequenceConfigBase : public UObject
{
	GENERATED_BODY()
public:
	UGenDialogueSequenceConfigBase(const FObjectInitializer& ObjectInitializer);

	UPROPERTY()
	UPreviewDialogueSoundSequence* PreviewDialogueSoundSequence;

public:
	UPROPERTY(EditAnywhere, Category = "站位模板", meta = (ShowOnlyInnerProperties = true))
	FDialogueStationInstance DialogueStation;

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool IsConfigValid() const;
public:
	virtual void GeneratePreview() const {}

	virtual void Generate(TSharedRef<ISequencer> SequencerRef, UWorld* World, const TMap<FName, TSoftObjectPtr<ACharacter>>& CharacterNameInstanceMap, UAutoGenDialogueSequence& AutoGenDialogueSequence) const {}
};
