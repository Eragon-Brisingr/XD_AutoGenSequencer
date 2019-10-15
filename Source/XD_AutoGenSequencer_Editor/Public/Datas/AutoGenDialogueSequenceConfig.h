// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenDialogueSequenceConfigBase.h"
#include "SubclassOf.h"
#include "AutoGenDialogueType.h"
#include "AutoGenDialogueSequenceConfig.generated.h"

class ADialogueStandPositionTemplate;
class USoundWave;
class ACharacter;
class UAnimSequence;
class UDialogueSentence;
class AAutoGenDialogueCameraTemplate;
class UAutoGenDialogueCameraSet;
class UPreviewDialogueSentenceSection;
struct FMovieSceneObjectBindingID;

/**
 *
 */
UCLASS(meta = (DisplayName = "默认生成配置"))
class XD_AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueSequenceConfig : public UGenDialogueSequenceConfigBase
{
	GENERATED_BODY()
public:
	UAutoGenDialogueSequenceConfig(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = "对白", meta = (DisplayName = "对白配置"))
	TArray<FDialogueSentenceEditData> DialogueSentenceEditDatas;

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	bool IsConfigValid(TArray<FText>& ErrorMessages) const override;
	TSubclassOf<UAutoGenDialogueAnimSetBase> GetAnimSetType() const override;
	bool IsDialogueSentenceEditDataValid(const FDialogueSentenceEditData &Data, const TArray<FName>& ValidNameList) const;

	//生成预览
public:
	//对白间隔时间
	UPROPERTY(EditAnywhere, Category = "生成预览配置", meta = (DisplayName = "对白间隔时间"))
	float PaddingTime = 0.6f;

	void GeneratePreview() const override;

	//生成对白
public:
	UPROPERTY(EditAnywhere, Category = "生成对白配置", meta = (DisplayName = "启用合并过短镜头"))
	uint8 bEnableMergeCamera : 1;
	//小于这个时间的镜头会被合并
	UPROPERTY(EditAnywhere, Category = "生成对白配置", meta = (DisplayName = "镜头合并时间阈值", EditCondition = "bEnableMergeCamera||bEnableSplitCamera"))
	float CameraMergeMaxTime = 2.5f;

	UPROPERTY(EditAnywhere, Category = "生成对白配置", meta = (DisplayName = "启用分割过长镜头"))
	uint8 bEnableSplitCamera : 1;
	//大于这个时间的镜头会尝试拆分
	UPROPERTY(EditAnywhere, Category = "生成对白配置", meta = (DisplayName = "镜头拆分时间阈值", EditCondition = "bEnableMergeCamera||bEnableSplitCamera"))
	float CameraSplitMinTime = 5.f;

	// 补光使用LightChannel2，角色想受光需要开启
	UPROPERTY(EditAnywhere, Category = "生成对白配置", meta = (DisplayName = "生成补光组"))
	uint8 bGenerateSupplementLightGroup : 1;

	UPROPERTY(EditAnywhere, Category = "生成对白配置", meta = (DisplayName = "输出生成日志"))
	uint8 bShowGenerateLog : 1;

	struct XD_AUTOGENSEQUENCER_EDITOR_API FGenDialogueData
	{
		UPreviewDialogueSentenceSection* PreviewDialogueSentenceSection;
		ACharacter* Speaker;
		TArray<ACharacter*> Targets;

		const FDialogueSentenceEditData& GetDialogueSentenceEditData() const;
		TRange<FFrameNumber> GetRange() const;
	};

	void Generate(TSharedRef<ISequencer> SequencerRef, UWorld* World, const TMap<FName, TSoftObjectPtr<ACharacter>>& CharacterNameInstanceMap, UAutoGenDialogueSystemData& AutoGenDialogueSystemData) const override;

	struct XD_AUTOGENSEQUENCER_EDITOR_API FAnimTrackData
	{
		struct XD_AUTOGENSEQUENCER_EDITOR_API FAnimSectionVirtualData
		{
			UAnimSequence* AnimSequence;
			TRange<FFrameNumber> AnimRange;
			float BlendInTime;
			float BlendOutTime;

			// 动画所持有的对话数据，考虑合并带来的多份数据
			TArray<const FGenDialogueData*> GenDialogueDatas;
			bool IsContainSpeakingData() const { return GenDialogueDatas.Num() > 0; }
		};
		TArray<FAnimSectionVirtualData> AnimSectionVirtualDatas;
	};
	using FAnimSectionVirtualData = FAnimTrackData::FAnimSectionVirtualData;
	virtual TMap<ACharacter*, FAnimTrackData> EvaluateAnimations(
		const FFrameNumber SequenceStartFrameNumber,
		const FFrameNumber SequenceEndFrameNumber,
		const FFrameRate FrameRate,
		const TArray<FGenDialogueData>& SortedDialogueDatas,
		const TMap<FName, ACharacter*>& NameInstanceMap,
		const TMap<ACharacter*, FGenDialogueCharacterData>& DialogueCharacterDataMap) const;

	virtual void EvaluateTalkAnimations(
		TMap<ACharacter*, FAnimTrackData>& AnimationTrackDataMap,
		const FFrameNumber SequenceStartFrameNumber,
		const FFrameNumber SequenceEndFrameNumber,
		const FFrameRate FrameRate,
		const TArray<FGenDialogueData>& SortedDialogueDatas,
		const TMap<FName, ACharacter*>& NameInstanceMap,
		const TMap<ACharacter*, FGenDialogueCharacterData>& DialogueCharacterDataMap) const;

	virtual UAnimSequence* EvaluateIdleAnimation(const TOptional<FAnimSectionVirtualData>& PrevTalkAnimData, const TOptional<FAnimSectionVirtualData>& NextTalkAnimData, FFrameRate FrameRate, const TRange<FFrameNumber>& IdleTimeRange, const FGenDialogueCharacterData& GenDialogueCharacterData) const;
};
