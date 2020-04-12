// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <UObject/NoExportTypes.h>
#include <UObject/SoftObjectPtr.h>
#include <Templates/SubclassOf.h>
#include <MovieSceneObjectBindingID.h>
#include "GenDialogueSequenceConfigBase.generated.h"

class ISequencer;
class ACharacter;
class UPreviewDialogueSoundSequence;
class UGenDialogueSequenceConfigBase;
class ADialogueStandPositionTemplate;
class UAutoGenDialogueAnimSetBase;
class UAutoGenDialogueCameraSet;
class ULevelSequence;
class UAutoGenDialogueCharacterSettings;
class UDialogueStandPositionTemplateAsset;

/**
 *
 */
USTRUCT()
struct XD_AUTOGENSEQUENCER_EDITOR_API FDialogueCharacterData
{
	GENERATED_BODY()
public:
	FDialogueCharacterData();

	UPROPERTY(EditAnywhere, meta = (DisplayName = "角色名"))
	FName NameOverride;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "实例引用"))
	TSoftObjectPtr<ACharacter> InstanceOverride;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "类型"))
	TSubclassOf<ACharacter> TypeOverride;
	
	UPROPERTY(EditAnywhere, meta = (DisplayName = "坐标覆盖"))
 	FTransform PositionOverride;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "对白动画集"))
	UAutoGenDialogueAnimSetBase* DialogueAnimSet;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "对白角色设置"))
	UAutoGenDialogueCharacterSettings* CharacterSettings;
};

// 生成期间的角色数据信息
struct XD_AUTOGENSEQUENCER_EDITOR_API FGenDialogueCharacterData : public FDialogueCharacterData
{
	FGenDialogueCharacterData(const FDialogueCharacterData& DialogueCharacterData)
		:FDialogueCharacterData(DialogueCharacterData)
	{}

	int32 CharacterIdx;
	FMovieSceneObjectBindingID BindingID;
	FTransform WorldPosition;
};

UCLASS(abstract)
class XD_AUTOGENSEQUENCER_EDITOR_API UGenDialogueSequenceConfigBase : public UObject
{
	GENERATED_BODY()
public:
	UGenDialogueSequenceConfigBase(const FObjectInitializer& ObjectInitializer);

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	UPROPERTY()
	UPreviewDialogueSoundSequence* PreviewDialogueSequence;

	UPROPERTY(EditAnywhere, Category = "1.对白角色配置", meta = (DisplayName = "站位模板"))
	UDialogueStandPositionTemplateAsset* DialogueStationTemplateAsset;
	ADialogueStandPositionTemplate* GetDialogueStationTemplate() const;

	UPROPERTY(EditAnywhere, Category = "1.对白角色配置", EditFixedSize = true, meta = (DisplayName = "对话角色"))
	TArray<FDialogueCharacterData> DialogueCharacterDatas;
	
	void SyncInstanceData(const ADialogueStandPositionTemplate* Instance);
	TArray<FName> GetCharacterNames() const;

	TArray<TSharedPtr<FName>> DialogueNameList;
	TArray<TSharedPtr<FName>>& GetDialogueNameList();
	void ReinitDialogueNameList();

	FORCEINLINE const TArray<FDialogueCharacterData>& GetDialogueCharacterDatas() const { return DialogueCharacterDatas; }
	const FDialogueCharacterData* FindDialogueCharacterData(const FName& Name) const;
	// 有效性检测
public:
	virtual bool IsConfigValid(TArray<FText>& ErrorMessages) const;
	// 允许的AnimSet类型
	virtual TSubclassOf<UAutoGenDialogueAnimSetBase> GetAnimSetType() const;
	// 允许的CharacterSettings类型
	virtual TSubclassOf<UAutoGenDialogueCharacterSettings> GetCharacterSettingsType() const;

	// 生成
public:
	// 生成预览导轨
	UFUNCTION(CallInEditor, Category = "2.生成预览配置", meta = (DisplayName = "生成预览序列"))
	void GeneratePreviewSequence();

	virtual void GeneratePreview(const TMap<FName, ACharacter*>& CharacterNameInstanceMap) const {}

	// 生成对白序列
	UFUNCTION(CallInEditor, Category = "3.生成对白配置", meta = (DisplayName = "生成对白序列"))
	void GenerateDialogueSequence();

	virtual void Generate(TSharedRef<ISequencer> SequencerRef, UWorld* World, const TMap<FName, ACharacter*>& CharacterNameInstanceMap, const FTransform& StandTemplateOrigin) const {}
protected:
	UAutoGenDialogueCameraSet* GetAutoGenDialogueCameraSet() const;

	// 编辑时接口
public:
	virtual void WhenCharacterNameChanged(const FName& OldName, const FName& NewName) {}
public:
	ULevelSequence* GetOwingLevelSequence() const;

	// TODO：考虑世界原点变换，考虑运行时
	UPROPERTY()
	FTransform StandPositionPosition = FTransform::Identity;
	FTransform GetStandPositionPosition() const { return StandPositionPosition; }

	UPROPERTY()
	uint8 bIsNewCreated : 1;
	UPROPERTY()
	uint8 bIsNotSetStandPosition : 1;
};
