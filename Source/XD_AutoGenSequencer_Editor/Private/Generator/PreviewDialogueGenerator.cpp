// Fill out your copyright notice in the Description page of Project Settings.


#include "PreviewDialogueGenerator.h"
#include "AutoGenDialogueSequenceConfig.h"
#include "PreviewDialogueSoundSequence.h"
#include "PreviewDialogueSentenceTrack.h"
#include "ScopedTransaction.h"
#include "MessageDialog.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

void FPreviewDialogueGenerator::Generate(const UAutoGenDialogueSequenceConfig* Config, UPreviewDialogueSoundSequence* PreviewDialogueSoundSequence)
{
	if (!Config->IsConfigValid())
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("预览导轨生成报错", "配置中存在问题，无法生成"));
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("生成预览序列描述", "生成预览序列"));
	PreviewDialogueSoundSequence->Modify();

	UMovieScene* MovieScene = PreviewDialogueSoundSequence->GetMovieScene();
	for (UPreviewDialogueSentenceTrack* PreviewDialogueSentenceTrack : PreviewDialogueSoundSequence->PreviewDialogueSentenceTracks)
	{
		if (PreviewDialogueSentenceTrack)
		{
			PreviewDialogueSoundSequence->GetMovieScene()->RemoveMasterTrack(*PreviewDialogueSentenceTrack);
		}
	}
	PreviewDialogueSoundSequence->PreviewDialogueSentenceTracks.Empty();

	const TArray<FDialogueSentenceEditData>& DialogueSentenceEditDatas = Config->DialogueSentenceEditDatas;

	FFrameRate FrameRate = MovieScene->GetTickResolution();

	FFrameNumber PaddingNumber = FFrameTime(PaddingTime * FrameRate).FrameNumber;
	FFrameNumber CurFrameNumber;
	TMap<FName, UPreviewDialogueSentenceTrack*> TrackMap;
	for (const FDialogueSentenceEditData& DialogueSentenceEditData : DialogueSentenceEditDatas)
	{
		FName SpeakerName = Config->GetSpeakerNameBySentence(DialogueSentenceEditData);
		if (SpeakerName != NAME_None)
		{
			UPreviewDialogueSentenceTrack* PreviewDialogueSentenceTrack = TrackMap.FindRef(SpeakerName);
			if (PreviewDialogueSentenceTrack == nullptr)
			{
				PreviewDialogueSentenceTrack = MovieScene->AddMasterTrack<UPreviewDialogueSentenceTrack>();
				PreviewDialogueSentenceTrack->SetDisplayName(FText::FromName(SpeakerName));
				PreviewDialogueSentenceTrack->SpeakerName = SpeakerName;
				PreviewDialogueSoundSequence->PreviewDialogueSentenceTracks.Add(PreviewDialogueSentenceTrack);
				TrackMap.Add(SpeakerName, PreviewDialogueSentenceTrack);
			}

			FFrameNumber Duration;
			UMovieSceneSection* Section = PreviewDialogueSentenceTrack->AddNewDialogueOnRow(DialogueSentenceEditData, CurFrameNumber, Duration);
			CurFrameNumber += Duration + PaddingNumber;
		}
	}

	FFrameNumber EndFrameNumber = (CurFrameNumber - PaddingNumber);
	MovieScene->SetPlaybackRange(FFrameNumber(0), EndFrameNumber.Value);
	MovieScene->SetWorkingRange(0.f, EndFrameNumber / FrameRate);
	MovieScene->SetViewRange(0.f, EndFrameNumber / FrameRate);
}

#undef LOCTEXT_NAMESPACE
