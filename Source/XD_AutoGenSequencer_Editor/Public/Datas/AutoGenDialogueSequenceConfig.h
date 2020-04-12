// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Datas/GenDialogueSequenceConfigBase.h"
#include <Templates/SubclassOf.h>
#include "Datas/AutoGenDialogueType.h"
#include "AutoGenDialogueSequenceConfig.generated.h"

class ADialogueStandPositionTemplate;
class USoundWave;
class ACharacter;
class UAnimSequence;
class UDialogueSentence;
class UDialogueSentenceTrack;
class AAutoGenDialogueCameraTemplate;
class UAutoGenDialogueCameraSet;
struct FMovieSceneObjectBindingID;
class UMovieSceneTrack;
class UDialogueSentenceSection;
class UMovieScene;

/**
 *
 */
UCLASS(meta = (DisplayName = "默认生成配置"))
class XD_AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueSequenceConfig : public UGenDialogueSequenceConfigBase
{
	GENERATED_BODY()
public:
	UAutoGenDialogueSequenceConfig(const FObjectInitializer& ObjectInitializer);

	// 生成预览
public:
	UPROPERTY(EditAnywhere, Category = "2.生成预览配置", meta = (DisplayName = "对白配置"))
	TArray<FDialogueSentenceEditData> DialogueSentenceEditDatas;

	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	bool IsConfigValid(TArray<FText>& ErrorMessages) const override;
	TSubclassOf<UAutoGenDialogueAnimSetBase> GetAnimSetType() const override;
	bool IsDialogueSentenceEditDataValid(const FDialogueSentenceEditData &Data, const TArray<FName>& ValidNameList, TArray<FText>& ErrorText) const;
	void WhenCharacterNameChanged(const FName& OldName, const FName& NewName) override;

	// 对白间隔时间
	UPROPERTY(EditAnywhere, Category = "2.生成预览配置", meta = (DisplayName = "对白间隔时间"))
	float PaddingTime = 0.6f;

	void GeneratePreview(const TMap<FName, ACharacter*>& CharacterNameInstanceMap) const final;

	virtual void PostGeneratePreviewSentence(UMovieScene& MovieScene, const TMap<FName, ACharacter*>& CharacterNameInstanceMap, const TMap<ACharacter*, FGenDialogueCharacterData>& DialogueCharacterDataMap, const TMap<ACharacter*, UDialogueSentenceTrack*>& DialogueSentenceTrackMap) const {}

	// 生成对白
public:
	UPROPERTY(EditAnywhere, Category = "3.生成对白配置", meta = (DisplayName = "合并过短镜头"))
	uint8 bEnableMergeCamera : 1;
	// 小于这个时间的镜头会被合并
	UPROPERTY(EditAnywhere, Category = "3.生成对白配置", meta = (DisplayName = "镜头合并时间阈值", EditCondition = "bEnableMergeCamera||bEnableSplitCamera"))
	float CameraMergeMaxTime = 2.5f;

	UPROPERTY(EditAnywhere, Category = "3.生成对白配置", meta = (DisplayName = "分割过长镜头"))
	uint8 bEnableSplitCamera : 1;
	// 大于这个时间的镜头会尝试拆分
	UPROPERTY(EditAnywhere, Category = "3.生成对白配置", meta = (DisplayName = "镜头拆分时间阈值", EditCondition = "bEnableMergeCamera||bEnableSplitCamera"))
	float CameraSplitMinTime = 5.f;
	
	UPROPERTY(EditAnywhere, Category = "3.生成对白配置", meta = (DisplayName = "合并相似镜头"))
	uint8 bEnableMergeSameCut : 1;
	UPROPERTY(EditAnywhere, Category = "3.生成对白配置", meta = (DisplayName = "相似镜头合并距离阈值", EditCondition = bEnableMergeSameCut))
	float SameCutMergeDistanceThreshold = 50.f;
	UPROPERTY(EditAnywhere, Category = "3.生成对白配置", meta = (DisplayName = "相似镜头合并角度阈值", EditCondition = bEnableMergeSameCut))
	float SameCutMergeAngleThreshold = 30.f;

	// 防止其它对话角色挡住当前相机注视角色
	UPROPERTY(EditAnywhere, Category = "3.生成对白配置", meta = (DisplayName = "避免镜头目标被遮挡"))
	uint8 bEnableAvoidCameraTargetCovered : 1;

	// 补光使用LightChannel2，角色想受光需要开启
	UPROPERTY(EditAnywhere, Category = "3.生成对白配置", meta = (DisplayName = "生成补光组"))
	uint8 bGenerateSupplementLightGroup : 1;

	UPROPERTY(EditAnywhere, Category = "3.生成对白配置", meta = (DisplayName = "输出生成日志"))
	uint8 bShowGenerateLog : 1;

	// 系统会根据结束时的动画姿态对选择集中的东湖进行打分，选择分数较高的动画
	// 打分的依据是骨骼的距离，键为骨骼名，值为打分权重
	UPROPERTY(EditAnywhere, Category = "3.生成对白配置", meta = (DisplayName = "动画生成关键骨骼"))
	TMap<FName, float> AnimBoneWeights;

	struct XD_AUTOGENSEQUENCER_EDITOR_API FGenDialogueData
	{
		UDialogueSentenceSection* PreviewDialogueSentenceSection;
		ACharacter* Speaker;
		TArray<ACharacter*> Targets;
	};

	void Generate(TSharedRef<ISequencer> SequencerRef, UWorld* World, const TMap<FName, ACharacter*>& CharacterNameInstanceMap, const FTransform& StandTemplateOrigin) const final;

protected:
	virtual void PostGeneratedDialogue(UMovieScene& MovieScene,
		const FFrameNumber SequenceStartFrameNumber,
		const FFrameNumber SequenceEndFrameNumber,
		const FFrameRate FrameRate,
		const TArray<FGenDialogueData>& SortedDialogueDatas,
		const TMap<FName, ACharacter*>& CharacterNameInstanceMap,
		const TMap<ACharacter*, FGenDialogueCharacterData>& DialogueCharacterDataMap) const {}

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

	virtual void EvaluateCharacterTalkAnimation(TArray<FAnimSectionVirtualData>& AnimSectionVirtualDatas, 
		FFrameNumber AnimStartFrameNumber, 
		FFrameNumber AnimEndFrameNumber, 
		const FFrameRate FrameRate, 
		const FGenDialogueCharacterData& DialogueCharacterData, 
		const FGenDialogueData& GenDialogueData) const;

	virtual void EvaluateIdleAnimation(TArray<FAnimSectionVirtualData>& IdleAnimDatas, 
		FAnimSectionVirtualData* PrevTalkAnimData, 
		FAnimSectionVirtualData* NextTalkAnimData, 
		FFrameRate FrameRate, 
		const TRange<FFrameNumber>& IdleTimeRange, 
		const FGenDialogueCharacterData& DialogueCharacterData) const;

	// 生成时用到的数据
protected:
	UPROPERTY()
	mutable TArray<UMovieSceneTrack*> AutoGenTracks;
	UPROPERTY()
	mutable TArray<FGuid> AutoGenCameraGuids;
	UPROPERTY()
	mutable TArray<FGuid> AutoGenCameraComponentGuids;
	UPROPERTY()
	mutable TArray<FGuid> AutoGenSupplementLightGuids;
};
