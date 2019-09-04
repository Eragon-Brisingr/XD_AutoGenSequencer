// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SubclassOf.h"
#include "AutoGenDialogueSequenceConfig.generated.h"

class ADialogueStandPositionTemplate;
class USoundWave;
class ACharacter;
class UAnimSequence;
class UDialogueSentence;
class AAutoGenDialogueCameraTemplate;


USTRUCT()
struct XD_AUTOGENSEQUENCER_API FDialogueStationInstanceOverride
{
	GENERATED_BODY()
public:
	FDialogueStationInstanceOverride() = default;

	UPROPERTY(EditAnywhere)
	FName NameOverride;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<ACharacter> InstanceOverride;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ACharacter> TypeOverride;
	
	UPROPERTY(EditAnywhere)
 	FTransform PositionOverride;
};

USTRUCT()
struct XD_AUTOGENSEQUENCER_API FDialogueStationInstance
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<ADialogueStandPositionTemplate> DialogueStationTemplate;

	UPROPERTY(EditAnywhere, EditFixedSize = true)
	TArray<FDialogueStationInstanceOverride> DialogueStationTemplateOverride;
	
#if WITH_EDITORONLY_DATA
	void SyncInstanceData(const ADialogueStandPositionTemplate* Instance);
	bool IsValid() const;
	TArray<FName> GetCharacterNames() const;

	TArray<TSharedPtr<FString>> DialogueNameList;
	TArray<TSharedPtr<FString>>& GetDialogueNameList();
	void ReinitDialogueNameList();
#endif
};

USTRUCT()
struct XD_AUTOGENSEQUENCER_API FDialogueCharacterName
{
	GENERATED_BODY()
public:
	FDialogueCharacterName(const FName& Name = NAME_None)
		:Name(Name)
	{}

	UPROPERTY(EditAnywhere)
	FName Name;

	friend bool operator==(const FDialogueCharacterName& LHS, const FDialogueCharacterName& RHS) { return LHS.Name == RHS.Name; }
	friend uint32 GetTypeHash(const FDialogueCharacterName& DialogueCharacterName) { return GetTypeHash(DialogueCharacterName.Name); }
	FORCEINLINE FName GetName() const { return Name; }
};

USTRUCT()
struct XD_AUTOGENSEQUENCER_API FDialogueSentenceEditData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	UDialogueSentence* DialogueSentence;
	UPROPERTY(EditAnywhere, meta = (DisplayName = "说话者"))
	FDialogueCharacterName SpeakerName = TEXT("Role");
	UPROPERTY(EditAnywhere, meta = (DisplayName = "向所有人说"))
	uint8 bToAllTargets : 1;
	UPROPERTY(EditAnywhere, meta = (DisplayName = "对话目标"))
	TArray<FDialogueCharacterName> TargetNames = { FDialogueCharacterName(TEXT("Target1")) };
	UPROPERTY(EditAnywhere)
	FName Emotion;
public:
	USoundBase* GetDefaultDialogueSound() const;
};

UCLASS()
class XD_AUTOGENSEQUENCER_API UAutoGenDialogueSequenceConfig : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "站位模板")
	FDialogueStationInstance DialogueStation;

	UPROPERTY(EditAnywhere, Category = "对话")
	TArray<FDialogueSentenceEditData> DialogueSentenceEditDatas;

	// 临时，要再设计，应该针对角色配置
	UPROPERTY(EditAnywhere)
	FName TalkAnimSlotName = TEXT("DefaultSlot");
	UPROPERTY(EditAnywhere, Category = "动作")
	TArray<UAnimSequence*> RandomAnims;
	UPROPERTY(EditAnywhere, Category = "动作")
	TArray<UAnimSequence*> IdleAnims;

	// 临时，要再设计
	UPROPERTY(EditAnywhere, Category = "镜头")
	TArray<TSubclassOf<AAutoGenDialogueCameraTemplate>> CameraTemplates;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	const FDialogueStationInstanceOverride* GetStationOverrideDataBySentence(const FDialogueSentenceEditData& DialogueSentenceEditData) const;
	FName GetSpeakerNameBySentence(const FDialogueSentenceEditData& DialogueSentenceEditData) const;
	bool IsConfigValid() const;
	bool IsDialogueSentenceEditDataValid(const FDialogueSentenceEditData &Data, const TArray<FName>& ValidNameList) const;
#endif
};
