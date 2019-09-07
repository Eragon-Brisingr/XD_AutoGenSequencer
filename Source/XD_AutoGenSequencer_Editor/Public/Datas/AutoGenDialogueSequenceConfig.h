// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenDialogueSequenceConfigBase.h"
#include "SubclassOf.h"
#include "AutoGenDialogueSequenceConfig.generated.h"

class ADialogueStandPositionTemplate;
class USoundWave;
class ACharacter;
class UAnimSequence;
class UDialogueSentence;
class AAutoGenDialogueCameraTemplate;
class UAutoGenDialogueCameraSet;

/**
 *
 */
USTRUCT()
struct XD_AUTOGENSEQUENCER_EDITOR_API FDialogueCharacterName
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
struct XD_AUTOGENSEQUENCER_EDITOR_API FDialogueSentenceEditData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	UDialogueSentence* DialogueSentence;
	UPROPERTY(EditAnywhere, meta = (DisplayName = "说话者"))
	FDialogueCharacterName SpeakerName = TEXT("Role");
	UPROPERTY(EditAnywhere, meta = (DisplayName = "向所有人说"))
	uint8 bToAllTargets : 1;
	UPROPERTY(EditAnywhere, meta = (DisplayName = "对白目标"))
	TArray<FDialogueCharacterName> TargetNames = { FDialogueCharacterName(TEXT("Target1")) };
	UPROPERTY(EditAnywhere)
	FName Emotion;
public:
	USoundBase* GetDefaultDialogueSound() const;
};

UCLASS(meta = (DisplayName = "默认生成配置"))
class XD_AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueSequenceConfig : public UGenDialogueSequenceConfigBase
{
	GENERATED_BODY()
public:
	UAutoGenDialogueSequenceConfig(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = "对白", meta = (DisplayName = "对白配置"))
	TArray<FDialogueSentenceEditData> DialogueSentenceEditDatas;

	UPROPERTY(EditAnywhere, Category = "镜头", meta = (DisplayName = "默认镜头集"))
	UAutoGenDialogueCameraSet* AutoGenDialogueCameraSet;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	bool IsConfigValid() const override;
	bool IsDialogueSentenceEditDataValid(const FDialogueSentenceEditData &Data, const TArray<FName>& ValidNameList) const;
#endif

	//生成预览
public:
	//对白间隔时间
	UPROPERTY(EditAnywhere, Category = "生成预览配置", meta = (DisplayName = "对白间隔时间"))
	float PaddingTime = 0.6f;

	void GeneratePreview() const override;

	//生成对白
public:
	// TODO：不要用同一个镜头太久时间？
// 	UPROPERTY(EditAnywhere, Category = "生成对白配置", meta = (DisplayName = "镜头最大使用次数"))
// 	int32 CameraMaxUseTimes = 3;
	UPROPERTY(EditAnywhere, Category = "生成对白配置", meta = (DisplayName = "最长单个镜头时间"))
	float CameraMergeMaxTime = 2.5f;
	// 必须大于CameraMergeMaxTime * 3
	UPROPERTY(EditAnywhere, Category = "生成对白配置", meta = (DisplayName = "最短镜头分割时间"))
	float CameraSplitMinTime = 12.f;

	void Generate(TSharedRef<ISequencer> SequencerRef, UWorld* World, const TMap<FName, TSoftObjectPtr<ACharacter>>& CharacterNameInstanceMap, UAutoGenDialogueSequence& AutoGenDialogueSequence) const override;
};
