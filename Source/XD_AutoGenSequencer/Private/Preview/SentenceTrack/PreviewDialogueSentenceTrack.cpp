// Fill out your copyright notice in the Description page of Project Settings.


#include "PreviewDialogueSentenceTrack.h"
#include "PreviewDialogueSentenceSection.h"
#include "AutoGenDialogueSequenceConfig.h"

#define LOCTEXT_NAMESPACE "DialogueSentenceTrack"

UPreviewDialogueSentenceTrack::UPreviewDialogueSentenceTrack()
{
#if WITH_EDITORONLY_DATA
	SetDisplayName(LOCTEXT("PreviewDialogueSentenceTrackName", "对话预览轨"));
	SetRowHeight(16.f);
#endif
}

bool UPreviewDialogueSentenceTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == UPreviewDialogueSentenceSection::StaticClass();
}

UMovieSceneSection* UPreviewDialogueSentenceTrack::CreateNewSection()
{
	return NewObject<UPreviewDialogueSentenceSection>(this, NAME_None, RF_Transactional);
}

UMovieSceneSection* UPreviewDialogueSentenceTrack::AddNewDialogueOnRow(const FDialogueSentenceEditData& DialogueSentenceEditData, FFrameNumber Time, FFrameNumber& DurationTime, int32 RowIndex)
{
	USoundBase* Sound = DialogueSentenceEditData.GetDefaultDialogueSound();
	check(Sound);

	FFrameRate FrameRate = GetTypedOuter<UMovieScene>()->GetTickResolution();
	FFrameTime DurationToUse = 1.f * FrameRate; // if all else fails, use 1 second duration

	float SoundDuration = MovieSceneHelpers::GetSoundDuration(Sound);
	if (SoundDuration != INDEFINITELY_LOOPING_DURATION)
	{
		DurationToUse = SoundDuration * FrameRate;
	}
	DurationTime = DurationToUse.FrameNumber;

	// add the section
	UPreviewDialogueSentenceSection* NewSection = NewObject<UPreviewDialogueSentenceSection>(this, NAME_None, RF_Transactional);
	NewSection->DialogueSentenceEditData = DialogueSentenceEditData;
	NewSection->InitialPlacementOnRow(GetAllSections(), Time, DurationToUse.FrameNumber.Value, RowIndex);
	NewSection->SetSound(Sound);

	AddSection(*NewSection);

	return NewSection;
}

bool UPreviewDialogueSentenceTrack::SupportsMultipleRows() const
{
	return false;
}

#undef LOCTEXT_NAMESPACE
