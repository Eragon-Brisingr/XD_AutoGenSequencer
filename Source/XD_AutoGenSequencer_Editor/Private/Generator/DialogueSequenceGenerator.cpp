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

void FDialogueSequenceGenerator::Generate(TSharedRef<ISequencer> SequencerRef, UWorld* World, const TMap<FName, TSoftObjectPtr<ACharacter>>& CharacterNameInstanceMap, const UPreviewDialogueSoundSequence* PreviewDialogueSoundSequence, UAutoGenDialogueSequence* AutoGenDialogueSequence)
{
	const FScopedTransaction Transaction(LOCTEXT("生成对话序列描述", "生成对话序列"));
	AutoGenDialogueSequence->Modify();

	const UAutoGenDialogueSequenceConfig& GenConfig = *AutoGenDialogueSequence->AutoGenDialogueSequenceConfig;

	ISequencer& Sequencer = SequencerRef.Get();
	UMovieScene* MovieScene = AutoGenDialogueSequence->GetMovieScene();
	FFrameRate FrameRate = MovieScene->GetTickResolution();
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
				FGenDialogueData Data;
				Data.PreviewDialogueSentenceSection = CastChecked<UPreviewDialogueSentenceSection>(Section);
				Data.SpeakerInstance = SpeakerInstance;
				Data.SpeakerName = SpeakerName;

				const FDialogueSentenceEditData& DialogueSentenceEditData = Data.PreviewDialogueSentenceSection->DialogueSentenceEditData;
				for (const FDialogueCharacterName& TargetName : DialogueSentenceEditData.TargetNames)
				{
					Data.Targets.Add(NameInstanceMap.FindRef(TargetName.GetName()));
				}

				SortedDialogueDatas.Add(Data);
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
	TMap<ACharacter*, UMovieSceneSkeletalAnimationTrack*> AnimationTrackTrackMap;
	FFrameNumber CurCameraCutFrame = FFrameNumber(0);
	for (int32 Idx = 0; Idx < SortedDialogueDatas.Num(); ++Idx)
	{
		FGenDialogueData& GenDialogueData = SortedDialogueDatas[Idx];
		const FName& SpeakerName = GenDialogueData.SpeakerName;
		const UPreviewDialogueSentenceSection* PreviewDialogueSentenceSection = GenDialogueData.PreviewDialogueSentenceSection;
		const FDialogueSentenceEditData& DialogueSentenceEditData = PreviewDialogueSentenceSection->DialogueSentenceEditData;
		TRange<FFrameNumber> SectionRange = PreviewDialogueSentenceSection->GetRange();
		FFrameNumber StartFrameNumber = SectionRange.GetLowerBoundValue();
		FFrameNumber EndFrameNumber = SectionRange.GetUpperBoundValue();
		ACharacter* Speaker = GenDialogueData.SpeakerInstance;
		const TArray<ACharacter*>& Targets = GenDialogueData.Targets;
		FGuid SpeakerBindingGuid = InstanceBindingIdMap[Speaker].GetGuid();

		// -- 对话
		{
			UDialogueSentenceTrack* DialogueSentenceTrack = DialogueSentenceTrackMap.FindRef(Speaker);
			if (!DialogueSentenceTrack)
			{
				DialogueSentenceTrack = MovieScene->AddTrack<UDialogueSentenceTrack>(SpeakerBindingGuid);
				DialogueSentenceTrackMap.Add(Speaker, DialogueSentenceTrack);
				AutoGenTracks.Add(DialogueSentenceTrack);
			}
			UDialogueSentenceSection* DialogueSentenceSection = DialogueSentenceTrack->AddNewSentenceOnRow(DialogueSentenceEditData.DialogueSentence, StartFrameNumber);
			for (ACharacter* Target : GenDialogueData.Targets)
			{
				DialogueSentenceSection->Targets.Add(InstanceBindingIdMap[Target]);
			}
		}

		// -- 动作
		{
			UMovieSceneSkeletalAnimationTrack* AnimationTrackTrack = AnimationTrackTrackMap.FindRef(Speaker);
			if (!AnimationTrackTrack)
			{
				AnimationTrackTrack = MovieScene->AddTrack<UMovieSceneSkeletalAnimationTrack>(SpeakerBindingGuid);
				AnimationTrackTrackMap.Add(Speaker, AnimationTrackTrack);
				AutoGenTracks.Add(AnimationTrackTrack);
			}
			if (GenConfig.RandomAnims.Num() > 0)
			{
				UMovieSceneSkeletalAnimationSection* SkeletalAnimationSection = Cast<UMovieSceneSkeletalAnimationSection>(AnimationTrackTrack->AddNewAnimation(StartFrameNumber, GenConfig.RandomAnims[FMath::RandHelper(GenConfig.RandomAnims.Num())]));
				SkeletalAnimationSection->SetRange(TRange<FFrameNumber>(StartFrameNumber, EndFrameNumber));

				const float BlendWidget = 0.5f;
				FFrameNumber BlendFrameNumber = (BlendWidget * FrameRate).GetFrame();

				SkeletalAnimationSection->Params.Weight.AddCubicKey(StartFrameNumber, 0.f);
				SkeletalAnimationSection->Params.Weight.AddCubicKey(StartFrameNumber + BlendFrameNumber, 1.f);
				SkeletalAnimationSection->Params.Weight.AddCubicKey(EndFrameNumber - BlendFrameNumber, 1.f);
				SkeletalAnimationSection->Params.Weight.AddCubicKey(EndFrameNumber, 0.f);
			}
		}

		// -- 相机
		{
			// TODO: 不用Spawn，增加Spawnable之后设置Template
			FActorSpawnParameters SpawnParams;
			SpawnParams.ObjectFlags &= ~RF_Transactional;
			ACineCameraActor* AutoGenCamera = World->SpawnActor<ACineCameraActor>(SpawnParams);
			AutoGenCamera->SetActorLabel(FString::Printf(TEXT("%s_Camera"), *SpeakerName.ToString()));
			{
				FRotator CameraRotation;
				FVector CameraLocation;
				Speaker->GetActorEyesViewPoint(CameraLocation, CameraRotation);
				CameraRotation.Yaw += FMath::RandRange(-30.f, 30.f);
				CameraLocation = CameraLocation - CameraRotation.Vector() * 100.f;
				AutoGenCamera->SetActorLocationAndRotation(CameraLocation, CameraRotation);
			}
			World->EditorDestroyActor(AutoGenCamera, false);

			FGuid CameraGuid = AutoGenDialogueSequence->CreateSpawnable(AutoGenCamera);
			AutoGenCameraFolder->AddChildObjectBinding(CameraGuid);
			AutoGenCamera = Cast<ACineCameraActor>(MovieScene->FindSpawnable(CameraGuid)->GetObjectTemplate());
			AutoGenDialogueSequence->AutoGenCameraGuids.Add(CameraGuid);
			{
				UMovieSceneCameraCutSection* NewSection = Cast<UMovieSceneCameraCutSection>(CameraCutTrack->CreateNewSection());
				NewSection->SetRange(TRange<FFrameNumber>(CurCameraCutFrame, EndFrameNumber));
				CurCameraCutFrame = EndFrameNumber;
				NewSection->SetCameraGuid(CameraGuid);
				CameraCutTrack->AddSection(*NewSection);

				UCineCameraComponent* CineCameraComponent = AutoGenCamera->GetCineCameraComponent();
				FGuid CineCameraComponentGuid = DialogueSequencerUtils::AddChildObject(AutoGenDialogueSequence, AutoGenCamera, MovieScene, CameraGuid, CineCameraComponent);
				AutoGenDialogueSequence->AutoGenCameraComponentGuids.Add(CineCameraComponentGuid);
				{
					if (Targets.Num() > 0)
					{
						ACharacter* Target = Targets[0];
						AAutoGenDialogueCameraTemplate* CameraTemplate = GenConfig.CameraTemplates[FMath::RandHelper(GenConfig.CameraTemplates.Num())].GetDefaultObject();

						bool InvertCameraPos = InstanceIdxMap[Speaker] - InstanceIdxMap[Target] > 0;
						UTwoTargetCameraTrackingTrack* TwoTargetCameraTrackingTrack = MovieScene->AddTrack<UTwoTargetCameraTrackingTrack>(CineCameraComponentGuid);
						UTwoTargetCameraTrackingSection* TwoTargetCameraTrackingSection = TwoTargetCameraTrackingTrack->AddNewSentenceOnRow(InstanceBindingIdMap[Target], InstanceBindingIdMap[Speaker]);
						TwoTargetCameraTrackingSection->CameraYaw.SetDefault(!InvertCameraPos ? CameraTemplate->CameraYawAngle : -CameraTemplate->CameraYawAngle);
						TwoTargetCameraTrackingSection->FrontTargetRate.SetDefault(CameraTemplate->FrontTargetRate);
						TwoTargetCameraTrackingSection->BackTargetRate.SetDefault(CameraTemplate->BackTargetRate);

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
	}

	// 后处理
	FFrameNumber EndFrameNumber = PreviewDialogueSoundSequence->GetMovieScene()->GetPlaybackRange().GetUpperBoundValue();
	MovieScene->SetPlaybackRange(FFrameNumber(0), EndFrameNumber.Value);
	MovieScene->SetWorkingRange(0.f, EndFrameNumber / FrameRate);
	MovieScene->SetViewRange(0.f, EndFrameNumber / FrameRate);
}

#undef LOCTEXT_NAMESPACE
