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

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

void FDialogueSequenceGenerator::Generate(TSharedRef<ISequencer> SequencerRef, UWorld* World, const TMap<FName, TSoftObjectPtr<ACharacter>>& CharacterNameInstanceMap, const UPreviewDialogueSoundSequence* PreviewDialogueSoundSequence, UAutoGenDialogueSequence* AutoGenDialogueSequence)
{
	const FScopedTransaction Transaction(LOCTEXT("生成对话序列描述", "生成对话序列"));
	AutoGenDialogueSequence->Modify();

	const UAutoGenDialogueSequenceConfig& GenConfig = *AutoGenDialogueSequence->AutoGenDialogueSequenceConfig;

	UMovieScene* MovieScene = AutoGenDialogueSequence->GetMovieScene();
	TArray<UMovieSceneTrack*>& AutoGenTracks = AutoGenDialogueSequence->AutoGenTracks;
	TArray<FGuid>& AutoGenCameraGuids = AutoGenDialogueSequence->AutoGenCameraGuids;
	
	// 数据预处理
	TMap<FName, ACharacter*> NameInstanceMap;
	TMap<UDialogueVoice*, TArray<ACharacter*>> DialogueVoiceInstanceMap;
	for (const TPair<FName, TSoftObjectPtr<ACharacter>>& Entry : CharacterNameInstanceMap)
	{
		if (ACharacter* Character = Entry.Value.Get())
		{
			if (Character->Implements<UDialogueInterface>())
			{
				if (UDialogueVoice* DialogueVoice = IDialogueInterface::GetDialogueVoice(Character))
				{
					NameInstanceMap.Add(Entry.Key, Character);
					DialogueVoiceInstanceMap.FindOrAdd(DialogueVoice).Add(Character);
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
		for (const FGuid& CameraGuid : AutoGenCameraGuids)
		{
			int32 Idx = CameraCutTrack->GetAllSections().IndexOfByPredicate([&](UMovieSceneSection* E) { return CastChecked<UMovieSceneCameraCutSection>(E)->GetCameraBindingID().GetGuid() == CameraGuid; });
			if (Idx != INDEX_NONE)
			{
				CameraCutTrack->RemoveSectionAt(Idx);
			}
			MovieScene->RemoveSpawnable(CameraGuid);

			AutoGenCameraFolder->RemoveChildObjectBinding(CameraGuid);
		}
		AutoGenCameraGuids.Empty();
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
				UDialogueWave* DialogueWave = DialogueSentenceEditData.DialogueWave;
				if (FDialogueContextMapping* DialogueContextMapping = DialogueWave->ContextMappings.FindByPredicate([&](const FDialogueContextMapping& E) {return E.Context.Speaker == DialogueSentenceEditData.GetDefualtDialogueSpeaker(); }))
				{
					for (UDialogueVoice* Target : DialogueContextMapping->Context.Targets)
					{
						Data.Targets.Append(DialogueVoiceInstanceMap.FindRef(Target));
					}
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
	TMap<ACharacter*, FGuid> InstanceGuidMap;
	for (const TPair<FName, ACharacter*>& Entry : NameInstanceMap)
	{
		FGuid BindingGuid = AutoGenDialogueSequence->FindOrAddPossessable(Entry.Value);
		InstanceGuidMap.Add(Entry.Value, BindingGuid);
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
		FGuid SpeakerBindingGuid = InstanceGuidMap[Speaker];

		// -- 对话
		{
			UDialogueSentenceTrack* DialogueSentenceTrack = DialogueSentenceTrackMap.FindRef(Speaker);
			if (!DialogueSentenceTrack)
			{
				DialogueSentenceTrack = MovieScene->AddTrack<UDialogueSentenceTrack>(SpeakerBindingGuid);
				DialogueSentenceTrackMap.Add(Speaker, DialogueSentenceTrack);
				AutoGenTracks.Add(DialogueSentenceTrack);
			}
			UDialogueSentenceSection* DialogueSentenceSection = DialogueSentenceTrack->AddNewSentenceOnRow(DialogueSentenceEditData.DialogueWave, StartFrameNumber);
			for (ACharacter* Target : GenDialogueData.Targets)
			{
				// TODO: 先用MovieSceneSequenceID::Root，以后再找MovieSceneSequenceID怎么获得
				DialogueSentenceSection->Targets.Add(FMovieSceneObjectBindingID(InstanceGuidMap[Target], MovieSceneSequenceID::Root));
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
			AnimationTrackTrack->AddNewAnimation(StartFrameNumber, GenConfig.RandomAnims[FMath::RandHelper(GenConfig.RandomAnims.Num())]);
		}

		// -- 相机
		{
			FRotator CameraRotation;
			FVector CameraLocation;
			Speaker->GetActorEyesViewPoint(CameraLocation, CameraRotation);
			CameraRotation.Yaw += FMath::RandRange(-30.f, 30.f);
			CameraLocation = CameraLocation - CameraRotation.Vector() * 100.f;

			FActorSpawnParameters SpawnParams;
			SpawnParams.ObjectFlags &= ~RF_Transactional;
			ACineCameraActor* AutoGenCamera = World->SpawnActor<ACineCameraActor>(CameraLocation, CameraRotation, SpawnParams);
			AutoGenCamera->SetActorLabel(FString::Printf(TEXT("%s_Camera"), *SpeakerName.ToString()));
			FGuid CameraGuid = AutoGenDialogueSequence->CreateSpawnable(AutoGenCamera);
			AutoGenDialogueSequence->AutoGenCameraGuids.Add(CameraGuid);
			AutoGenCameraFolder->AddChildObjectBinding(CameraGuid);
			{
				UMovieSceneCameraCutSection* NewSection = Cast<UMovieSceneCameraCutSection>(CameraCutTrack->CreateNewSection());
				NewSection->SetRange(TRange<FFrameNumber>(CurCameraCutFrame, EndFrameNumber));
				CurCameraCutFrame = EndFrameNumber;
				NewSection->SetCameraGuid(CameraGuid);
				CameraCutTrack->AddSection(*NewSection);
			}
			World->EditorDestroyActor(AutoGenCamera, false);
		}
	}

	// 后处理
	FFrameRate FrameRate = MovieScene->GetTickResolution();
	FFrameNumber EndFrameNumber = PreviewDialogueSoundSequence->GetMovieScene()->GetPlaybackRange().GetUpperBoundValue();
	MovieScene->SetPlaybackRange(FFrameNumber(0), EndFrameNumber.Value);
	MovieScene->SetWorkingRange(0.f, EndFrameNumber / FrameRate);
	MovieScene->SetViewRange(0.f, EndFrameNumber / FrameRate);
}

#undef LOCTEXT_NAMESPACE
