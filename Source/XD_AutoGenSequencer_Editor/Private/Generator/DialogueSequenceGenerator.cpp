// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueSequenceGenerator.h"

#include "AutoGenDialogueSequence.h"
#include "PreviewDialogueSoundSequence.h"
#include "AutoGenDialogueSequenceConfig.h"
#include "PreviewDialogueSentenceTrack.h"
#include "PreviewDialogueSentenceSection.h"
#include "DialogueSentenceTrack.h"
#include "DialogueInterface.h"
#include "DialogueSentenceSection.h"
#include "TwoTargetCameraTrackingTrack.h"
#include "AutoGenDialogueCameraTemplate.h"
#include "TwoTargetCameraTrackingSection.h"
#include "DialogueCameraUtils.h"

#include "ScopedTransaction.h"
#include "GameFramework/Character.h"
#include "MovieSceneCameraCutTrack.h"
#include "Engine/World.h"
#include "CinematicCamera/Public/CineCameraActor.h"
#include "MovieSceneToolHelpers.h"
#include "MovieSceneFolder.h"
#include "MovieSceneCameraCutSection.h"
#include "Sound/DialogueWave.h"
#include "MovieSceneSkeletalAnimationTrack.h"
#include "CineCameraComponent.h"
#include "MovieSceneFloatTrack.h"
#include "MovieSceneFloatSection.h"
#include "Animation/AnimSequence.h"
#include "MovieSceneObjectPropertyTrack.h"
#include "MovieSceneObjectPropertySection.h"
#include "MovieSceneSkeletalAnimationSection.h"
#include "MovieSceneSpawnTrack.h"
#include "MovieSceneBoolSection.h"
#include "Transform.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

namespace DialogueSequencerUtils
{
	template<typename TrackType, typename SectionType>
	static SectionType* CreatePropertyTrack(UMovieScene* MovieScene, const FGuid& ObjectGuid, const FName& PropertyName, const FString& PropertyPath)
	{
		TrackType* Track = MovieScene->AddTrack<TrackType>(ObjectGuid);
		Track->SetPropertyNameAndPath(PropertyName, PropertyPath);
		SectionType* Section = Cast<SectionType>(Track->CreateNewSection());
		Track->AddSection(*Section);
		Section->SetRange(TRange<FFrameNumber>::All());
		return Section;
	}

	static FGuid AddChildObject(UMovieSceneSequence* Sequence, UObject* Parent, UMovieScene* MovieScene, const FGuid& ParentGuid, UObject* ChildObject)
	{
		FGuid ChildGuid = MovieScene->AddPossessable(ChildObject->GetName(), ChildObject->GetClass());
		FMovieScenePossessable* ChildPossessable = MovieScene->FindPossessable(ChildGuid);
		ChildPossessable->SetParent(ParentGuid);
		FMovieSceneSpawnable* ParentSpawnable = MovieScene->FindSpawnable(ParentGuid);
		ParentSpawnable->AddChildPossessable(ChildGuid);
		Sequence->BindPossessableObject(ChildGuid, *ChildObject, Parent);
		return ChildGuid;
	}
}

void FDialogueSequenceGenerator::Generate(TSharedRef<ISequencer> SequencerRef, UWorld* World, const TMap<FName, TSoftObjectPtr<ACharacter>>& CharacterNameInstanceMap, 
	const UAutoGenDialogueSequenceConfig& GenConfig, const UPreviewDialogueSoundSequence* PreviewDialogueSoundSequence, UAutoGenDialogueSequence* AutoGenDialogueSequence)
{
	const FScopedTransaction Transaction(LOCTEXT("生成对话序列描述", "生成对话序列"));
	AutoGenDialogueSequence->Modify();

	ISequencer& Sequencer = SequencerRef.Get();
	UMovieScene* MovieScene = AutoGenDialogueSequence->GetMovieScene();
	const FFrameRate FrameRate = MovieScene->GetTickResolution();
	const FFrameNumber SequenceStartFrameNumber = FFrameNumber(0);
	const FFrameNumber SequenceEndFrameNumber = PreviewDialogueSoundSequence->GetMovieScene()->GetPlaybackRange().GetUpperBoundValue();
	TArray<UMovieSceneTrack*>& AutoGenTracks = AutoGenDialogueSequence->AutoGenTracks;
	
	// 数据预处理
	TMap<FName, ACharacter*> NameInstanceMap;
	TMap<ACharacter*, int32> InstanceIdxMap;
	{
		int32 Idx = 0;
		for (const TPair<FName, TSoftObjectPtr<ACharacter>>& Entry : CharacterNameInstanceMap)
		{
			if (ACharacter* Character = Entry.Value.Get())
			{
				if (Character->Implements<UDialogueInterface>())
				{
					NameInstanceMap.Add(Entry.Key, Character);
					InstanceIdxMap.Add(Character, Idx);
					Idx += 1;
				}
			}
		}
	}

	UMovieSceneCameraCutTrack* CameraCutTrack = Cast<UMovieSceneCameraCutTrack>(MovieScene->GetCameraCutTrack());
	if (CameraCutTrack == nullptr)
	{
		CameraCutTrack = (UMovieSceneCameraCutTrack*)MovieScene->AddCameraCutTrack(UMovieSceneCameraCutTrack::StaticClass());
		CameraCutTrack->Modify();
	}

	UMovieSceneFolder* AutoGenCameraFolder;
	{
		const FName AutoGenCameraFolderName = TEXT("自动相机组");
		if (UMovieSceneFolder** P_AutoGenCameraFolder = MovieScene->GetRootFolders().FindByPredicate([&](UMovieSceneFolder* Folder) {return Folder->GetFolderName() == AutoGenCameraFolderName; }))
		{
			AutoGenCameraFolder = *P_AutoGenCameraFolder;
		}
		else
		{
			AutoGenCameraFolder = NewObject<UMovieSceneFolder>(MovieScene, NAME_None, RF_Transactional);
			MovieScene->GetRootFolders().Add(AutoGenCameraFolder);
			AutoGenCameraFolder->SetFolderName(AutoGenCameraFolderName);
		}
	}

	// 清除之前的数据
	{
		for (UMovieSceneTrack* PreGenTrack : AutoGenTracks)
		{
			if (PreGenTrack)
			{
				MovieScene->RemoveTrack(*PreGenTrack);
			}
		}
		AutoGenTracks.Empty();
		for (const FGuid& CameraComponentGuid : AutoGenDialogueSequence->AutoGenCameraComponentGuids)
		{
			MovieScene->RemovePossessable(CameraComponentGuid);
		}
		AutoGenDialogueSequence->AutoGenCameraComponentGuids.Empty();
		for (const FGuid& CameraGuid : AutoGenDialogueSequence->AutoGenCameraGuids)
		{
			int32 Idx = CameraCutTrack->GetAllSections().IndexOfByPredicate([&](UMovieSceneSection* E) { return CastChecked<UMovieSceneCameraCutSection>(E)->GetCameraBindingID().GetGuid() == CameraGuid; });
			if (Idx != INDEX_NONE)
			{
				CameraCutTrack->RemoveSectionAt(Idx);
			}
			MovieScene->RemoveSpawnable(CameraGuid);

			AutoGenCameraFolder->RemoveChildObjectBinding(CameraGuid);
		}
		AutoGenDialogueSequence->AutoGenCameraGuids.Empty();
	}

	// 整理数据
	struct FGenDialogueData
	{
		UPreviewDialogueSentenceSection* PreviewDialogueSentenceSection;
		FName SpeakerName;
		ACharacter* SpeakerInstance;
		TArray<ACharacter*> Targets;
	};

	TArray<FGenDialogueData> SortedDialogueDatas;
	for (UPreviewDialogueSentenceTrack* PreviewDialogueSentenceTrack : PreviewDialogueSoundSequence->PreviewDialogueSentenceTracks)
	{
		const FName& SpeakerName = PreviewDialogueSentenceTrack->SpeakerName;
		if (ACharacter* SpeakerInstance = NameInstanceMap.FindRef(SpeakerName))
		{
			for (UMovieSceneSection* Section : PreviewDialogueSentenceTrack->GetAllSections())
			{
				FGenDialogueData& Data = SortedDialogueDatas.AddDefaulted_GetRef();
				Data.PreviewDialogueSentenceSection = CastChecked<UPreviewDialogueSentenceSection>(Section);
				Data.SpeakerInstance = SpeakerInstance;
				Data.SpeakerName = SpeakerName;

				const FDialogueSentenceEditData& DialogueSentenceEditData = Data.PreviewDialogueSentenceSection->DialogueSentenceEditData;
				for (const FDialogueCharacterName& TargetName : DialogueSentenceEditData.TargetNames)
				{
					Data.Targets.Add(NameInstanceMap.FindRef(TargetName.GetName()));
				}
			}
		}
	}
	SortedDialogueDatas.Sort([&](const FGenDialogueData& LHS, const FGenDialogueData& RHS)
		{
			return LHS.PreviewDialogueSentenceSection->GetRange().GetLowerBoundValue().Value < RHS.PreviewDialogueSentenceSection->GetRange().GetLowerBoundValue().Value;
		});

	// 生成对象轨
	TMap<ACharacter*, FMovieSceneObjectBindingID> InstanceBindingIdMap;
	for (const TPair<FName, ACharacter*>& Entry : NameInstanceMap)
	{
		FGuid BindingGuid = AutoGenDialogueSequence->FindOrAddPossessable(Entry.Value);
		// TODO: 先用MovieSceneSequenceID::Root，以后再找MovieSceneSequenceID怎么获得
		InstanceBindingIdMap.Add(Entry.Value, FMovieSceneObjectBindingID(BindingGuid, MovieSceneSequenceID::Root));
	}

	// 生成导轨
	TMap<ACharacter*, UDialogueSentenceTrack*> DialogueSentenceTrackMap;

	// -- 对话
	for (int32 Idx = 0; Idx < SortedDialogueDatas.Num(); ++Idx)
	{
		FGenDialogueData& GenDialogueData = SortedDialogueDatas[Idx];
		ACharacter* Speaker = GenDialogueData.SpeakerInstance;
		FGuid SpeakerBindingGuid = InstanceBindingIdMap[Speaker].GetGuid();
		const UPreviewDialogueSentenceSection* PreviewDialogueSentenceSection = GenDialogueData.PreviewDialogueSentenceSection;
		const FDialogueSentenceEditData& DialogueSentenceEditData = PreviewDialogueSentenceSection->DialogueSentenceEditData;
		TRange<FFrameNumber> SectionRange = PreviewDialogueSentenceSection->GetRange();
		FFrameNumber StartFrameNumber = SectionRange.GetLowerBoundValue();
		FFrameNumber EndFrameNumber = SectionRange.GetUpperBoundValue();
		const TArray<ACharacter*>& Targets = GenDialogueData.Targets;

		{
			UDialogueSentenceTrack* DialogueSentenceTrack = DialogueSentenceTrackMap.FindRef(Speaker);
			if (!DialogueSentenceTrack)
			{
				DialogueSentenceTrack = MovieScene->AddTrack<UDialogueSentenceTrack>(SpeakerBindingGuid);
				DialogueSentenceTrackMap.Add(Speaker, DialogueSentenceTrack);
				AutoGenTracks.Add(DialogueSentenceTrack);
			}
			UDialogueSentenceSection* DialogueSentenceSection = DialogueSentenceTrack->AddNewSentenceOnRow(DialogueSentenceEditData.DialogueSentence, StartFrameNumber);
			for (ACharacter* Target : Targets)
			{
				DialogueSentenceSection->Targets.Add(InstanceBindingIdMap[Target]);
			}
		}
	}

	// -- 动作
	struct FAnimTrackUtils
	{
		static FFrameNumber SecondToFrameNumber(float Second, const FFrameRate FrameRate)
		{
			return (Second * FrameRate).GetFrame();
		}

		static void SetBlendInOutValue(FMovieSceneFloatChannel& AnimWeight, const FFrameRate FrameRate, FFrameNumber StartFrameNumber, float StartBlendTime, FFrameNumber EndFrameNumber, float EndBlendTime)
		{
			SetBlendInValue(AnimWeight, FrameRate, StartFrameNumber, StartBlendTime, 0.f, 1.f);
			SetBlendOutValue(AnimWeight, FrameRate, EndFrameNumber, StartBlendTime, 1.f, 0.f);
		}

		static void SetBlendInValue(FMovieSceneFloatChannel& AnimWeight, const FFrameRate FrameRate, FFrameNumber StartFrameNumber, float BlendTime, float StartWeight, float EndWeight)
		{
			const FFrameNumber BlendFrameNumber = SecondToFrameNumber(BlendTime, FrameRate);
			AnimWeight.AddCubicKey(StartFrameNumber, StartWeight);
			AnimWeight.AddCubicKey(StartFrameNumber + BlendFrameNumber, EndWeight);
		}

		static void SetBlendOutValue(FMovieSceneFloatChannel& AnimWeight, const FFrameRate FrameRate, FFrameNumber EndFrameNumber, float BlendTime, float StartWeight, float EndWeight)
		{
			const FFrameNumber BlendFrameNumber = SecondToFrameNumber(BlendTime, FrameRate);
			AnimWeight.AddCubicKey(EndFrameNumber - BlendFrameNumber, StartWeight);
			AnimWeight.AddCubicKey(EndFrameNumber, EndWeight);
		}
	};

	struct FAnimTrackData
	{
		struct FAnimSectionData
		{
			UMovieSceneSkeletalAnimationSection* Section;
			float BlendInTime;
			float BlendOutTime;
		};

		UMovieSceneSkeletalAnimationTrack* Track;
		TArray<FAnimSectionData> TalkAnimSections;
	};
	TMap<ACharacter*, FAnimTrackData> AnimationTrackDataMap;
	for (int32 Idx = 0; Idx < SortedDialogueDatas.Num(); ++Idx)
	{
		FGenDialogueData& GenDialogueData = SortedDialogueDatas[Idx];
		ACharacter* Speaker = GenDialogueData.SpeakerInstance;
		FGuid SpeakerBindingGuid = InstanceBindingIdMap[Speaker].GetGuid();
		const UPreviewDialogueSentenceSection* PreviewDialogueSentenceSection = GenDialogueData.PreviewDialogueSentenceSection;
		TRange<FFrameNumber> SectionRange = PreviewDialogueSentenceSection->GetRange();
		FFrameNumber StartFrameNumber = SectionRange.GetLowerBoundValue();
		FFrameNumber EndFrameNumber = SectionRange.GetUpperBoundValue();

		FAnimTrackData& AnimationTrackData = AnimationTrackDataMap.FindOrAdd(Speaker);
		UMovieSceneSkeletalAnimationTrack*& AnimTrack = AnimationTrackData.Track;

		if (!AnimTrack)
		{
			AnimTrack = MovieScene->AddTrack<UMovieSceneSkeletalAnimationTrack>(SpeakerBindingGuid);
			AutoGenTracks.Add(AnimTrack);
		}
		if (GenConfig.RandomAnims.Num() > 0)
		{
			bool bIsSameSpeaker = Idx > 0 && SortedDialogueDatas[Idx - 1].SpeakerInstance == Speaker;
			// 假如是上一个对话者和现在相同则会合并动画
			if (!bIsSameSpeaker)
			{
				const float BlendTime = 0.25f;

				FFrameNumber AllSentenceEndTime = EndFrameNumber;
				for (int32 NextIdx = Idx + 1; NextIdx < SortedDialogueDatas.Num() && SortedDialogueDatas[NextIdx].SpeakerInstance == Speaker; ++NextIdx)
				{
					AllSentenceEndTime = SortedDialogueDatas[NextIdx].PreviewDialogueSentenceSection->GetRange().GetUpperBoundValue();
				}

				UMovieSceneSkeletalAnimationSection* TalkAnimSection = Cast<UMovieSceneSkeletalAnimationSection>(AnimTrack->AddNewAnimation(StartFrameNumber, GenConfig.RandomAnims[FMath::RandHelper(GenConfig.RandomAnims.Num())]));
				TalkAnimSection->SetRange(TRange<FFrameNumber>(StartFrameNumber, AllSentenceEndTime));

				TalkAnimSection->Params.SlotName = GenConfig.TalkAnimSlotName;
				FAnimTrackUtils::SetBlendInOutValue(TalkAnimSection->Params.Weight, FrameRate, StartFrameNumber, BlendTime, AllSentenceEndTime, BlendTime);

				FAnimTrackData::FAnimSectionData AnimSectionData;
				AnimSectionData.BlendInTime = BlendTime;
				AnimSectionData.BlendOutTime = BlendTime;
				AnimSectionData.Section = TalkAnimSection;
				AnimationTrackData.TalkAnimSections.Add(AnimSectionData);
			}
		}
	}
	// 动画后处理
	if (GenConfig.IdleAnims.Num())
	{
		for (TPair<ACharacter*, FAnimTrackData>& Pair : AnimationTrackDataMap)
		{
			FAnimTrackData& AnimTrackData = Pair.Value;
			if (AnimTrackData.TalkAnimSections.Num() > 0)
			{
				const TRange<FFrameNumber>& StartTalkAnimRange = AnimTrackData.TalkAnimSections[0].Section->GetRange();
				const TRange<FFrameNumber>& EndTalkAnimRange = AnimTrackData.TalkAnimSections.Last().Section->GetRange();

				float BlendTime = 0.25f;
				FFrameNumber BlendFrameNumber = FAnimTrackUtils::SecondToFrameNumber(BlendTime, FrameRate);

				UMovieSceneSkeletalAnimationSection* IdleSection = Cast<UMovieSceneSkeletalAnimationSection>(AnimTrackData.Track->AddNewAnimation(SequenceStartFrameNumber, GenConfig.IdleAnims[FMath::RandHelper(GenConfig.IdleAnims.Num())]));
				IdleSection->SetRange(TRange<FFrameNumber>(SequenceStartFrameNumber, SequenceEndFrameNumber));

				IdleSection->Params.SlotName = GenConfig.TalkAnimSlotName;
				FMovieSceneFloatChannel& IdleAnimWeight = IdleSection->Params.Weight;
				const bool IsSequenceFirstAnim = StartTalkAnimRange.Contains(SequenceStartFrameNumber + BlendFrameNumber);
				const bool IsSequenceLastAnim = EndTalkAnimRange.Contains(SequenceEndFrameNumber - BlendFrameNumber);
				if (!IsSequenceFirstAnim)
				{
					FAnimTrackUtils::SetBlendInValue(IdleAnimWeight, FrameRate, SequenceStartFrameNumber, BlendTime, 0.f, 1.f);
				}
				if (!IsSequenceLastAnim)
				{
					FAnimTrackUtils::SetBlendOutValue(IdleAnimWeight, FrameRate, SequenceEndFrameNumber, BlendTime, 1.f, 0.f);
				}

				for (int32 Idx = 0; Idx < AnimTrackData.TalkAnimSections.Num(); ++Idx)
				{
					FAnimTrackData::FAnimSectionData& TalkAnimSection = AnimTrackData.TalkAnimSections[Idx];
					const TRange<FFrameNumber>& TalkAnimRange = TalkAnimSection.Section->GetRange();
					if (Idx != 0 || !IsSequenceFirstAnim)
					{
						FAnimTrackUtils::SetBlendInValue(IdleAnimWeight, FrameRate, TalkAnimRange.GetLowerBoundValue(), TalkAnimSection.BlendInTime, 1.f, 0.f);
					}
					if (Idx != AnimTrackData.TalkAnimSections.Num() - 1 || !IsSequenceLastAnim)
					{
						FAnimTrackUtils::SetBlendOutValue(IdleAnimWeight, FrameRate, TalkAnimRange.GetUpperBoundValue(), TalkAnimSection.BlendOutTime, 0.f, 1.f);
					}
				}
			}
		}
	}

	// -- 相机
	struct FCameraCutUtils
	{
		static UMovieSceneCameraCutSection* AddCameraCut(UMovieScene* MovieScene, FGuid CameraGuid, UMovieSceneCameraCutTrack* CameraCutTrack, FFrameNumber CameraCutStartFrame, FFrameNumber CameraCutEndFrame)
		{
			UMovieSceneCameraCutSection* CameraCutSection = Cast<UMovieSceneCameraCutSection>(CameraCutTrack->CreateNewSection());
			CameraCutSection->SetRange(TRange<FFrameNumber>(CameraCutStartFrame, CameraCutEndFrame));
			CameraCutSection->SetCameraGuid(CameraGuid);
			CameraCutTrack->AddSection(*CameraCutSection);

			TMovieSceneChannelData<bool> SpawnChannel = GetSpawnChannel(MovieScene, CameraGuid);
			SpawnChannel.AddKey(CameraCutStartFrame, true);
			SpawnChannel.AddKey(CameraCutEndFrame, false);
			return CameraCutSection;
		}

		static TMovieSceneChannelData<bool> GetSpawnChannel(UMovieScene* MovieScene, FGuid CameraGuid)
		{
			UMovieSceneSpawnTrack* SpawnTrack = MovieScene->FindTrack<UMovieSceneSpawnTrack>(CameraGuid);
			UMovieSceneBoolSection* SpawnSection = Cast<UMovieSceneBoolSection>(SpawnTrack->GetAllSections()[0]);
			return SpawnSection->GetChannel().GetData();
		}
	};

	using FCameraHandle = int32;
	const FCameraHandle InvalidCameraHandle = INDEX_NONE;
	// 镜头的生成信息
	struct FCameraCutCreateData
	{
		FCameraHandle CameraHandle;
		TRange<FFrameNumber> CameraCutRange;
		TArray<int32> RelatedSentenceIdxs;
		float GetDuration(const FFrameRate& FrameRate) const
		{
			return (CameraCutRange / FrameRate).Size<double>();
		}
	};
	struct FCameraActorCreateData
	{
		FCameraHandle CameraHandle;
		int32 UseNumber;

		FVector CameraLocation;
		FRotator CameraRotation;
		ACineCameraActor* CameraTemplate;
		AAutoGenDialogueCameraTemplate* TwoTargetCameraTemplate;
		ACharacter* Speaker;
		FName SpeakerName;
		TArray<ACharacter*> Targets;
	};
	TArray<FCameraCutCreateData> CameraCutCreateDatas;
	TArray<FCameraActorCreateData> CameraActorCreateDatas;
	FFrameNumber CurCameraCutFrame = FFrameNumber(0);

	struct FCameraWeightsData
	{
		AAutoGenDialogueCameraTemplate* TwoTargetCameraTemplate;
		FVector CameraLocation;
		FRotator CameraRotation;
		float Weights;
	};

	struct FCamreaGenerateUtils
	{
		// 合并相同镜头
		static bool CanMergeSameCut(const FCameraCutCreateData& LHS, const FCameraCutCreateData& RHS)
		{
			return LHS.CameraHandle == RHS.CameraHandle;
		}
		static void MergeSameCut(int32 Idx, TArray<FCameraCutCreateData>& CameraCutCreateDatas, TArray<FCameraActorCreateData>& CameraActorCreateDatas)
		{
			FCameraCutCreateData& CameraCutCreateData = CameraCutCreateDatas[Idx];
			FCameraCutCreateData& NextCameraCutCreateData = CameraCutCreateDatas[Idx + 1];

			check(CanMergeSameCut(CameraCutCreateData, NextCameraCutCreateData));

			FCameraActorCreateData& CameraActorCreateData = CameraActorCreateDatas[CameraCutCreateData.CameraHandle];
			CameraActorCreateData.UseNumber -= 1;

			NextCameraCutCreateData.RelatedSentenceIdxs.Append(CameraCutCreateData.RelatedSentenceIdxs);
			NextCameraCutCreateData.RelatedSentenceIdxs.Sort();
			NextCameraCutCreateData.CameraCutRange.SetLowerBoundValue(CameraCutCreateData.CameraCutRange.GetLowerBoundValue());
			CameraCutCreateDatas.RemoveAt(Idx);
		}
		static void TryMergeAllSameCut(TArray<FCameraCutCreateData>& CameraCutCreateDatas, TArray<FCameraActorCreateData>& CameraActorCreateDatas)
		{
			for (int32 Idx = 0; Idx < CameraCutCreateDatas.Num() - 1;)
			{
				FCameraCutCreateData& CameraCutCreateData = CameraCutCreateDatas[Idx];
				FCameraCutCreateData& NextCameraCutCreateData = CameraCutCreateDatas[Idx + 1];
				if (CanMergeSameCut(CameraCutCreateData, NextCameraCutCreateData))
				{
					MergeSameCut(Idx, CameraCutCreateDatas, CameraActorCreateDatas);
				}
				else
				{
					++Idx;
				}
			}
		}

		// TODO：添加镜头选择策略
		static TArray<FCameraWeightsData> EvaluateCameraTemplate(const TArray<AAutoGenDialogueCameraTemplate*>& CameraTemplates, ACharacter* Speaker, const TArray<ACharacter*>& Targets, float DialogueProgress)
		{
			TArray<FCameraWeightsData> CameraWeightsDatas;
			// 选择对话的第一目标作为镜头目标
			if (ACharacter* Target = Targets.Num() > 0 ? Targets[0] : nullptr)
			{
				for (AAutoGenDialogueCameraTemplate* TwoTargetCameraTemplate : CameraTemplates)
				{
					FCameraWeightsData& CameraWeightsData = CameraWeightsDatas.AddDefaulted_GetRef();

					FVector SpeakerLocation = Speaker->GetPawnViewLocation();
					FVector TargetLocation = Target->GetPawnViewLocation();

					FVector CameraLocation;
					FRotator CameraRotation;
					FVector FocusCenterLocation;
					FDialogueCameraUtils::CameraTrackingTwoTargets(TwoTargetCameraTemplate->CameraYawAngle, TwoTargetCameraTemplate->FrontTargetRate, TwoTargetCameraTemplate->BackTargetRate,
						SpeakerLocation, TargetLocation, TwoTargetCameraTemplate->CineCameraComponent->FieldOfView, CameraLocation, CameraRotation, FocusCenterLocation);

					CameraWeightsData.TwoTargetCameraTemplate = TwoTargetCameraTemplate;
					CameraWeightsData.CameraLocation = CameraLocation;
					CameraWeightsData.CameraRotation = CameraRotation;

					float DistanceDialogueProgressWeights = FMath::Abs(0.5f - DialogueProgress) * 2.f;
					float CameraDistance = (CameraLocation - FocusCenterLocation).Size();
					float CharacterDistance = (SpeakerLocation - TargetLocation).Size();
					float DistanceWeights = FMath::Clamp(CameraDistance / (CharacterDistance * 2.f), 0.f, 1.f);

					CameraWeightsData.Weights = 1.f - FMath::Abs(DistanceWeights - DistanceDialogueProgressWeights);
				}
				CameraWeightsDatas.Sort([](const FCameraWeightsData& LHS, const FCameraWeightsData& RHS) {return LHS.Weights > RHS.Weights; });
			}
			// TODO：无目标时使用别的镜头
			else
			{
				checkNoEntry();
			}
			return CameraWeightsDatas;
		}

		static FCameraHandle AddOrFindVirtualCamreaData(float DialogueProgress, const UAutoGenDialogueSequenceConfig &GenConfig, const FName& SpeakerName, ACharacter* Speaker, const TArray<ACharacter*>& Targets, TArray<FCameraActorCreateData>& CameraActorCreateDatas)
		{
			TArray<AAutoGenDialogueCameraTemplate*> CameraTemplates;
			Algo::Transform(GenConfig.CameraTemplates, CameraTemplates, [](const TSubclassOf<AAutoGenDialogueCameraTemplate>& E) {return E.GetDefaultObject(); });
			TArray<FCameraWeightsData> CameraWeightsDatas = FCamreaGenerateUtils::EvaluateCameraTemplate(CameraTemplates, Speaker, Targets, DialogueProgress);

			// 现在直接选择最优的
			const FCameraWeightsData& SelectedData = CameraWeightsDatas[0];

			// 若镜头存在则直接使用
			FCameraActorCreateData* ExistedCameraData = CameraActorCreateDatas.FindByPredicate([&](const FCameraActorCreateData& E)
				{
					return E.Speaker == Speaker && 
						E.Targets == Targets && 
						E.CameraLocation == SelectedData.CameraLocation && 
						E.CameraRotation == SelectedData.CameraRotation && 
						E.TwoTargetCameraTemplate == SelectedData.TwoTargetCameraTemplate;
				});

			if (ExistedCameraData)
			{
				ExistedCameraData->UseNumber += 1;
				return ExistedCameraData->CameraHandle;
			}
			else
			{
				FCameraHandle CameraHandle = CameraActorCreateDatas.AddDefaulted();
				FCameraActorCreateData& CameraActorCreateData = CameraActorCreateDatas[CameraHandle];

				CameraActorCreateData.CameraHandle = CameraHandle;
				CameraActorCreateData.Speaker = Speaker;
				CameraActorCreateData.Targets = Targets;
				CameraActorCreateData.UseNumber = 1;

				AAutoGenDialogueCameraTemplate* TwoTargetCameraTemplate = SelectedData.TwoTargetCameraTemplate;
				CameraActorCreateData.CameraLocation = SelectedData.CameraLocation;
				CameraActorCreateData.CameraRotation = SelectedData.CameraRotation;
				CameraActorCreateData.CameraTemplate = CastChecked<ACineCameraActor>(TwoTargetCameraTemplate->CineCamera->GetChildActorTemplate());
				CameraActorCreateData.TwoTargetCameraTemplate = TwoTargetCameraTemplate;
				return CameraHandle;
			}
		}
	};

	const float DialogueCount = SortedDialogueDatas.Num();
	for (int32 Idx = 0; Idx < SortedDialogueDatas.Num(); ++Idx)
	{
		FGenDialogueData& GenDialogueData = SortedDialogueDatas[Idx];
		const TArray<ACharacter*>& Targets = GenDialogueData.Targets;
		ACharacter* Speaker = GenDialogueData.SpeakerInstance;
		const FName& SpeakerName = GenDialogueData.SpeakerName;
		const UPreviewDialogueSentenceSection* PreviewDialogueSentenceSection = GenDialogueData.PreviewDialogueSentenceSection;
		TRange<FFrameNumber> SectionRange = PreviewDialogueSentenceSection->GetRange();
		FFrameNumber EndFrameNumber = SectionRange.GetUpperBoundValue();

		const float DialogueProgress = Idx / DialogueCount;

		if (GenConfig.CameraTemplates.Num() > 0)
		{
			FCameraHandle CameraHandle = InvalidCameraHandle;

			CameraHandle = FCamreaGenerateUtils::AddOrFindVirtualCamreaData(DialogueProgress, GenConfig, SpeakerName, Speaker, Targets, CameraActorCreateDatas);

			FCameraCutCreateData& CameraCutCreateData = CameraCutCreateDatas.AddDefaulted_GetRef();
			CameraCutCreateData.CameraHandle = CameraHandle;
			CameraCutCreateData.CameraCutRange = TRange<FFrameNumber>(CurCameraCutFrame, EndFrameNumber);
			CameraCutCreateData.RelatedSentenceIdxs.Add(Idx);

			CurCameraCutFrame = EndFrameNumber;
		}
	}

	// 太短的镜头做合并
	for (int32 Idx = 0; Idx < CameraCutCreateDatas.Num(); ++Idx)
	{
		FCameraCutCreateData& CameraCutCreateData = CameraCutCreateDatas[Idx];
		float MidDuration = CameraCutCreateData.GetDuration(FrameRate);
		if (MidDuration < CameraMergeMaxTime)
		{
			// 合并方案
			// 假如合并后的镜头的长度超过了镜头分离最小长度就不合并
			// TODO：或许这种情况可以制定特殊的分离镜头策略
			const int32 MergeModeCount = 2;
			enum class ECameraMergeMode : uint8
			{
				// 1. 左侧拉长至右侧
				LeftToRight = 0,
				// 2. 右侧拉长至左侧
				RightToLeft = 1
			};

			TArray<ECameraMergeMode> InvalidModes;
			bool HasLeftCut = Idx != 0;
			bool HasRightCut = Idx != CameraCutCreateDatas.Num() - 1;

			if (!HasLeftCut)
			{
				InvalidModes.Add(ECameraMergeMode::LeftToRight);
			}
			else
			{
				FCameraCutCreateData& LeftCameraCutCreateData = CameraCutCreateDatas[Idx - 1];
				float MergedDuration = LeftCameraCutCreateData.GetDuration(FrameRate) + MidDuration;
				if (HasRightCut)
				{
					FCameraCutCreateData& RightCameraCutCreateData = CameraCutCreateDatas[Idx + 1];
					if (FCamreaGenerateUtils::CanMergeSameCut(LeftCameraCutCreateData, RightCameraCutCreateData))
					{
						MergedDuration += RightCameraCutCreateData.GetDuration(FrameRate);
					}
				}
				if (MergedDuration > CameraSplitMinTime)
				{
					InvalidModes.Add(ECameraMergeMode::LeftToRight);
				}
			}
			if (!HasRightCut)
			{
				InvalidModes.Add(ECameraMergeMode::RightToLeft);
			}
			else
			{
				FCameraCutCreateData& RightCameraCutCreateData = CameraCutCreateDatas[Idx + 1];
				float MergedDuration = RightCameraCutCreateData.GetDuration(FrameRate) + MidDuration;
				if (HasLeftCut)
				{
					FCameraCutCreateData& LeftCameraCutCreateData = CameraCutCreateDatas[Idx - 1];
					if (FCamreaGenerateUtils::CanMergeSameCut(LeftCameraCutCreateData, RightCameraCutCreateData))
					{
						MergedDuration += LeftCameraCutCreateData.GetDuration(FrameRate);
					}
				}
				if (MergedDuration > CameraSplitMinTime)
				{
					InvalidModes.Add(ECameraMergeMode::RightToLeft);
				}
			}

			// 假如没有可行的合并方案就不合并
			if (InvalidModes.Num() == MergeModeCount)
			{
				continue;
			}

			ECameraMergeMode CameraMergeMode;
			do
			{
				CameraMergeMode = ECameraMergeMode(FMath::RandHelper(MergeModeCount));
			} while (InvalidModes.Contains(CameraMergeMode));
			

			switch (CameraMergeMode)
			{
			case ECameraMergeMode::LeftToRight:
			{
				FCameraCutCreateData& LeftCameraCutCreateData = CameraCutCreateDatas[Idx - 1];
				LeftCameraCutCreateData.CameraCutRange.SetUpperBoundValue(CameraCutCreateData.CameraCutRange.GetUpperBoundValue());
			}
			break;
			case ECameraMergeMode::RightToLeft:
			{
				FCameraCutCreateData& RightCameraCutCreateData = CameraCutCreateDatas[Idx + 1];
				RightCameraCutCreateData.CameraCutRange.SetLowerBoundValue(CameraCutCreateData.CameraCutRange.GetLowerBoundValue());
			}
			break;
			}

			CameraCutCreateDatas.RemoveAt(Idx);
			Idx -= 1;
		}
	}

	FCamreaGenerateUtils::TryMergeAllSameCut(CameraCutCreateDatas, CameraActorCreateDatas);

	// 太长的镜头拆开来
	for (int32 Idx = 0; Idx < CameraCutCreateDatas.Num(); ++Idx)
	{
		FCameraCutCreateData& CameraCutCreateData = CameraCutCreateDatas[Idx];
		FCameraActorCreateData& CameraActorCreateData = CameraActorCreateDatas[CameraCutCreateData.CameraHandle];
		if (CameraCutCreateData.GetDuration(FrameRate) > CameraSplitMinTime)
		{
			FCameraHandle MidCameraHandle = InvalidCameraHandle;

			//假如为正反打镜头
			if (CameraActorCreateData.CameraTemplate)
			{
				FCameraActorCreateData* ExistedCameraCreateData = CameraActorCreateDatas.FindByPredicate([&](const FCameraActorCreateData& E)
					{
						// 尝试从已有的镜头中找个合适的反打镜头
						if (!E.TwoTargetCameraTemplate)
						{
							return false;
						}
						if (E.UseNumber >= CameraMaxUseTimes)
						{
							return false;
						}
						ACharacter* Target = CameraActorCreateData.Speaker;
						ACharacter* Speaker = CameraActorCreateData.Targets[0];
						return E.Speaker == Speaker && E.Targets[0] == Target;
					});
				if (ExistedCameraCreateData)
				{
					ExistedCameraCreateData->UseNumber += 1;

					MidCameraHandle = ExistedCameraCreateData->CameraHandle;
				}
			}

			if (MidCameraHandle == InvalidCameraHandle)
			{
				ACharacter* Speaker = CameraActorCreateData.Speaker;

				// 走通用镜头生成流程
				float DialogueProgress = CameraCutCreateData.RelatedSentenceIdxs[CameraCutCreateData.RelatedSentenceIdxs.Num() / 2] / DialogueCount;
				MidCameraHandle = FCamreaGenerateUtils::AddOrFindVirtualCamreaData(DialogueProgress, GenConfig, CameraActorCreateData.SpeakerName, Speaker, CameraActorCreateData.Targets, CameraActorCreateDatas);
			}

			// TODO：分离镜头后维持RelatedSentenceIdxs
			FCameraCutCreateData& LeftCameraCutCreateData = CameraCutCreateData;
			FFrameNumber LeftStartFrameNumber = LeftCameraCutCreateData.CameraCutRange.GetLowerBoundValue();
			FFrameNumber LeftEndFrameNumber = LeftCameraCutCreateData.CameraCutRange.GetUpperBoundValue();

			FFrameNumber MinPadding = (CameraMergeMaxTime * FrameRate).GetFrame();

			FFrameNumber MidStartFrameNumber = LeftStartFrameNumber + MinPadding;
			FFrameNumber MidEndFrameNumber = MidStartFrameNumber + MinPadding;
			FCameraCutCreateData& MidCameraCutCreateData = CameraCutCreateDatas.InsertDefaulted_GetRef(Idx + 1);
			MidCameraCutCreateData.CameraHandle = MidCameraHandle;
			MidCameraCutCreateData.CameraCutRange = TRange<FFrameNumber>(MidStartFrameNumber, MidEndFrameNumber);

			FFrameNumber RightStartFrameNumber = MidEndFrameNumber;
			FFrameNumber RightEndFrameNumber = LeftEndFrameNumber;
			FCameraCutCreateData& RightCameraCutCreateData = CameraCutCreateDatas.InsertDefaulted_GetRef(Idx + 2);
			RightCameraCutCreateData.CameraHandle = MidCameraHandle;
			RightCameraCutCreateData.CameraCutRange = TRange<FFrameNumber>(RightStartFrameNumber, RightEndFrameNumber);

			LeftCameraCutCreateData.CameraCutRange.SetUpperBoundValue(MidStartFrameNumber);
		}
	}

	FCamreaGenerateUtils::TryMergeAllSameCut(CameraCutCreateDatas, CameraActorCreateDatas);

	// 生成摄像机导轨
	TMap<FCameraHandle, FGuid> SpawnedCameraMap;
	for (const FCameraCutCreateData& CameraCutCreateData : CameraCutCreateDatas)
	{
		const FCameraActorCreateData& CameraActorCreateData = CameraActorCreateDatas[CameraCutCreateData.CameraHandle];
		const TRange<FFrameNumber>& CameraCutRange = CameraCutCreateData.CameraCutRange;

		FGuid& CameraGuid = SpawnedCameraMap.FindOrAdd(CameraCutCreateData.CameraHandle);
		if (CameraGuid.IsValid())
		{
			FCameraCutUtils::AddCameraCut(MovieScene, CameraGuid, CameraCutTrack, CameraCutRange.GetLowerBoundValue(), CameraCutRange.GetUpperBoundValue());
		}
		else
		{
			ACharacter* Speaker = CameraActorCreateData.Speaker;
			const TArray<ACharacter*>& Targets = CameraActorCreateData.Targets;

			// TODO: 不用Spawn，增加Spawnable之后设置Template
			FActorSpawnParameters SpawnParams;
			SpawnParams.ObjectFlags &= ~RF_Transactional;
			SpawnParams.Template = CameraActorCreateData.CameraTemplate;
			ACineCameraActor* AutoGenCamera = World->SpawnActor<ACineCameraActor>(CameraActorCreateData.CameraLocation, CameraActorCreateData.CameraRotation, SpawnParams);
			AutoGenCamera->SetActorLabel(FString::Printf(TEXT("%s_Camera"), *CameraActorCreateData.SpeakerName.ToString()));
			World->EditorDestroyActor(AutoGenCamera, false);
			CameraGuid = AutoGenDialogueSequence->CreateSpawnable(AutoGenCamera);

			AutoGenCameraFolder->AddChildObjectBinding(CameraGuid);
			AutoGenCamera = Cast<ACineCameraActor>(MovieScene->FindSpawnable(CameraGuid)->GetObjectTemplate());
			AutoGenDialogueSequence->AutoGenCameraGuids.Add(CameraGuid);
			{
				if (CurCameraCutFrame != SequenceStartFrameNumber)
				{
					TMovieSceneChannelData<bool> SpawnChannel = FCameraCutUtils::GetSpawnChannel(MovieScene, CameraGuid);
					SpawnChannel.AddKey(SequenceStartFrameNumber, false);
				}

				FCameraCutUtils::AddCameraCut(MovieScene, CameraGuid, CameraCutTrack, CameraCutRange.GetLowerBoundValue(), CameraCutRange.GetUpperBoundValue());

				UCineCameraComponent* CineCameraComponent = AutoGenCamera->GetCineCameraComponent();
				FGuid CineCameraComponentGuid = DialogueSequencerUtils::AddChildObject(AutoGenDialogueSequence, AutoGenCamera, MovieScene, CameraGuid, CineCameraComponent);
				AutoGenDialogueSequence->AutoGenCameraComponentGuids.Add(CineCameraComponentGuid);
				{
					// 假如是双目标追踪镜头则生成对应的导轨
					if (AAutoGenDialogueCameraTemplate* CameraTemplate = CameraActorCreateData.TwoTargetCameraTemplate)
					{
						check(Targets.Num() > 0);
						ACharacter* Target = Targets.Num() > 0 ? Targets[0] : nullptr;

						bool InvertCameraPos = InstanceIdxMap[Speaker] - InstanceIdxMap[Target] > 0;
						UTwoTargetCameraTrackingTrack* TwoTargetCameraTrackingTrack = MovieScene->AddTrack<UTwoTargetCameraTrackingTrack>(CineCameraComponentGuid);
						UTwoTargetCameraTrackingSection* TwoTargetCameraTrackingSection = TwoTargetCameraTrackingTrack->AddNewSentenceOnRow(InstanceBindingIdMap[Target], InstanceBindingIdMap[Speaker]);
						TwoTargetCameraTrackingSection->CameraYaw.SetDefault(!InvertCameraPos ? CameraTemplate->CameraYawAngle : -CameraTemplate->CameraYawAngle);
						TwoTargetCameraTrackingSection->FrontTargetRate.SetDefault(CameraTemplate->FrontTargetRate);
						TwoTargetCameraTrackingSection->BackTargetRate.SetDefault(CameraTemplate->BackTargetRate);
					}

					// 景深目标为说话者
					CineCameraComponent->FocusSettings.FocusMethod = ECameraFocusMethod::Tracking;
					// TODO: 现在用字符串记录路径，可以考虑使用FPropertyPath，或者编译期通过一种方式做检查
					UMovieSceneObjectPropertySection* ActorToTrackSection = DialogueSequencerUtils::CreatePropertyTrack<UMovieSceneObjectPropertyTrack, UMovieSceneObjectPropertySection>(MovieScene, CineCameraComponentGuid,
						GET_MEMBER_NAME_CHECKED(FCameraTrackingFocusSettings, ActorToTrack), TEXT("FocusSettings.TrackingFocusSettings.ActorToTrack"));
					FMovieSceneObjectPathChannel& ObjectChannel = ActorToTrackSection->ObjectChannel;
					ObjectChannel.SetDefault(Speaker);
				}
			}
		}
	}

	// 后处理
	MovieScene->SetPlaybackRange(FFrameNumber(0), SequenceEndFrameNumber.Value);
	MovieScene->SetWorkingRange(0.f, SequenceEndFrameNumber / FrameRate);
	MovieScene->SetViewRange(0.f, SequenceEndFrameNumber / FrameRate);
}

#undef LOCTEXT_NAMESPACE
