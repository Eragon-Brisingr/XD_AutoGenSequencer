// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SubclassOf.h"
#include "AutoGenDialogueSequenceConfig.generated.h"

class UDialogueWave;
class ADialogueStandPositionTemplate;
class USoundWave;
class UDialogueVoice;
class ACharacter;
class UAnimSequence;


USTRUCT()
struct XD_AUTOGENSEQUENCER_API FDialogueStationInstanceOverride
{
	GENERATED_BODY()
public:
	FDialogueStationInstanceOverride() = default;

	UPROPERTY(EditAnywhere)
	UDialogueVoice* DialogueVoiceOverride;

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

	void SyncInstanceData(const ADialogueStandPositionTemplate* Instance);
	bool IsValid() const;
};

USTRUCT()
struct XD_AUTOGENSEQUENCER_API FDialogueSentenceEditData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	UDialogueWave* DialogueWave;
public:
	USoundWave* GetDefaultDialogueSound() const;
	UDialogueVoice* GetDefualtDialogueSpeaker() const;
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

	// 临时，要再设计
	UPROPERTY(EditAnywhere, Category = "对话")
	TArray<UAnimSequence*> RandomAnims;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	const FDialogueStationInstanceOverride* GetStationOverrideDataBySentence(const FDialogueSentenceEditData& DialogueSentenceEditData) const;
	FName GetSpeakerNameBySentence(const FDialogueSentenceEditData& DialogueSentenceEditData) const;
	bool IsConfigValid() const;
};
