// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueSequenceGenerator.h"

#include "AutoGenDialogueSequence.h"
#include "PreviewDialogueSoundSequence.h"
#include "PreviewDialogueSentenceTrack.h"
#include "PreviewDialogueSentenceSection.h"
#include "DialogueSentenceTrack.h"

#include "ScopedTransaction.h"
#include "GameFramework/Character.h"
#include "MovieSceneCameraCutTrack.h"
#include "Engine/World.h"
#include "CinematicCamera/Public/CineCameraActor.h"
#include "MovieSceneToolHelpers.h"
#include "MovieSceneFolder.h"
#include "MovieSceneCameraCutSection.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

void FDialogueSequenceGenerator::Generate(TSharedRef<ISequencer> SequencerRef, UWorld* World, const TMap<FName, TSoftObjectPtr<ACharacter>>& CharacterNameInstanceMap, const UPreviewDialogueSoundSequence* PreviewDialogueSoundSequence, UAutoGenDialogueSequence* AutoGenDialogueSequence)
{
	const FScopedTransaction Transaction(LOCTEXT("生成对话序列描述", "生成对话序列"));
	AutoGenDialogueSequence->Modify();

	UMovieScene* MovieScene = AutoGenDialogueSequence->GetMovieScene();
	TArray<UMovieSceneTrack*>& AutoGenTracks = AutoGenDialogueSequence->AutoGenTracks;
	TArray<FGuid>& AutoGenCameraGuids = AutoGenDialogueSequence->AutoGenCameraGuids;

	TMap<FName, ACharacter*> NameInstanceMap;
	for (const TPair<FName, TSoftObjectPtr<ACharacter>>& Entry : CharacterNameInstanceMap)
	{
		if (ACharacter* Character = Entry.Value.Get())
		{
			NameInstanceMap.Add(Entry.Key, Character);
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
				SortedDialogueDatas.Add(Data);
			}
		}
	}
	SortedDialogueDatas.Sort([&](const FGenDialogueData& LHS, const FGenDialogueData& RHS)
		{
			return LHS.PreviewDialogueSentenceSection->GetRange().GetLowerBoundValue().Value < RHS.PreviewDialogueSentenceSection->GetRange().GetLowerBoundValue().Value;
		});

	// 开始生成
	TMap<ACharacter*, UDialogueSentenceTrack*> DialogueSentenceTrackMap;
	FFrameNumber CurCameraCutFrame = FFrameNumber(0);
	for (int32 Idx = 0; Idx < SortedDialogueDatas.Num(); ++Idx)
	{
		FGenDialogueData& GenDialogueData = SortedDialogueDatas[Idx];
		const FName& SpeakerName = GenDialogueData.SpeakerName;
		const UPreviewDialogueSentenceSection* PreviewDialogueSentenceSection = GenDialogueData.PreviewDialogueSentenceSection;
		const FDialogueSentenceEditData& DialogueSentenceEditData = PreviewDialogueSentenceSection->DialogueSentenceEditData;
		TRange<FFrameNumber> SectionRange = PreviewDialogueSentenceSection->GetRange();
		ACharacter* SpeakerInstance = GenDialogueData.SpeakerInstance;

		// -- 对话
		UDialogueSentenceTrack* DialogueSentenceTrack = DialogueSentenceTrackMap.FindRef(SpeakerInstance);
		if (!DialogueSentenceTrack)
		{
			FGuid BindingGuid = AutoGenDialogueSequence->FindOrAddPossessable(SpeakerInstance);
			DialogueSentenceTrack = MovieScene->AddTrack<UDialogueSentenceTrack>(BindingGuid);
			DialogueSentenceTrackMap.Add(SpeakerInstance, DialogueSentenceTrack);
			AutoGenTracks.Add(DialogueSentenceTrack);
		}
		DialogueSentenceTrack->AddNewSentenceOnRow(DialogueSentenceEditData.DialogueWave, SectionRange.GetLowerBoundValue());

		// -- 动作

		// -- 相机
		FRotator CameraRotation = SpeakerInstance->GetActorRotation();
		CameraRotation.Yaw += FMath::RandRange(-30.f, 30.f);
		FVector CameraLocation = SpeakerInstance->GetActorLocation() - CameraRotation.Vector() * 100.f;

		FActorSpawnParameters SpawnParams;
		SpawnParams.ObjectFlags &= ~RF_Transactional;
		ACineCameraActor* AutoGenCamera = World->SpawnActor<ACineCameraActor>(CameraLocation, CameraRotation, SpawnParams);
		AutoGenCamera->SetActorLabel(FString::Printf(TEXT("%s_Camera"), *SpeakerName.ToString()));
		FGuid CameraGuid = AutoGenDialogueSequence->CreateSpawnable(AutoGenCamera);
		AutoGenDialogueSequence->AutoGenCameraGuids.Add(CameraGuid);
		AutoGenCameraFolder->AddChildObjectBinding(CameraGuid);
		{
			UMovieSceneCameraCutSection* NewSection = Cast<UMovieSceneCameraCutSection>(CameraCutTrack->CreateNewSection());
			FFrameNumber EndFrameNumber = SectionRange.GetUpperBoundValue().Value;
			NewSection->SetRange(TRange<FFrameNumber>(CurCameraCutFrame, EndFrameNumber));
			CurCameraCutFrame = EndFrameNumber;
			NewSection->SetCameraGuid(CameraGuid);
			CameraCutTrack->AddSection(*NewSection);
		}
		World->EditorDestroyActor(AutoGenCamera, false);
	}

	// 后处理
	FFrameRate FrameRate = MovieScene->GetTickResolution();
	FFrameNumber EndFrameNumber = PreviewDialogueSoundSequence->GetMovieScene()->GetPlaybackRange().GetUpperBoundValue();
	MovieScene->SetPlaybackRange(FFrameNumber(0), EndFrameNumber.Value);
	MovieScene->SetWorkingRange(0.f, EndFrameNumber / FrameRate);
	MovieScene->SetViewRange(0.f, EndFrameNumber / FrameRate);
}

#undef LOCTEXT_NAMESPACE
