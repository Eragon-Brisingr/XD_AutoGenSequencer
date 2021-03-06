// Fill out your copyright notice in the Description page of Project Settings.


#include "Tracks/SentenceTrack/DialogueSentenceTrack.h"
#include <Compilation/MovieSceneCompilerRules.h>
#include <MovieSceneCommonHelpers.h>

#include "Tracks/SentenceTrack/DialogueSentenceSection.h"
#include "Data/DialogueSentence.h"

#define LOCTEXT_NAMESPACE "DialogueSentenceTrack"

UDialogueSentenceTrack::UDialogueSentenceTrack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	SetDisplayName(LOCTEXT("DialogueSentenceTrackName", "对白轨"));
#endif
}

bool UDialogueSentenceTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass->IsChildOf(UDialogueSentenceSection::StaticClass());
}

void UDialogueSentenceTrack::RemoveAllAnimationData()
{
	SentenceSections.Empty();
}

bool UDialogueSentenceTrack::HasSection(const UMovieSceneSection& Section) const
{
	return SentenceSections.Contains(&Section);
}

void UDialogueSentenceTrack::AddSection(UMovieSceneSection& Section)
{
	SentenceSections.Add(&Section);
}

void UDialogueSentenceTrack::RemoveSection(UMovieSceneSection& Section)
{
	SentenceSections.Remove(&Section);
}

void UDialogueSentenceTrack::RemoveSectionAt(int32 SectionIndex)
{
	SentenceSections.RemoveAt(SectionIndex);
}

bool UDialogueSentenceTrack::IsEmpty() const
{
	return SentenceSections.Num() == 0;
}

const TArray<UMovieSceneSection*>& UDialogueSentenceTrack::GetAllSections() const
{
	return SentenceSections;
}

bool UDialogueSentenceTrack::SupportsMultipleRows() const
{
	return false;
}

FMovieSceneTrackRowSegmentBlenderPtr UDialogueSentenceTrack::GetRowSegmentBlender() const
{
	struct FBlender : FMovieSceneTrackRowSegmentBlender
	{
		virtual void Blend(FSegmentBlendData& BlendData) const override
		{
			// Run the default high pass filter for overlap priority
			MovieSceneSegmentCompiler::FilterOutUnderlappingSections(BlendData);
		}
	};
	return FBlender();
}

UMovieSceneSection* UDialogueSentenceTrack::CreateNewSection()
{
	return NewObject<UDialogueSentenceSection>(this, UDialogueSentenceSection::StaticClass(), NAME_None, RF_Transactional);
}

UDialogueSentenceSection* UDialogueSentenceTrack::AddNewSentenceOnRow(UDialogueSentence* DialogueSentence, FFrameNumber Time, int32 RowIndex /*= INDEX_NONE*/)
{
	check(DialogueSentence && DialogueSentence->SentenceWave);

	FFrameRate FrameRate = GetTypedOuter<UMovieScene>()->GetTickResolution();
	FFrameTime DurationToUse = 1.f * FrameRate; // if all else fails, use 1 second duration

	float SoundDuration = DialogueSentence->GetDuration();
	DurationToUse = SoundDuration * FrameRate;

	// add the section
	UDialogueSentenceSection* NewSection = NewObject<UDialogueSentenceSection>(this, DialogueSentence->GetSectionImplType(), NAME_None, RF_Transactional);
	NewSection->DialogueSentence = DialogueSentence;
	NewSection->InitialPlacementOnRow(GetAllSections(), Time, DurationToUse.FrameNumber.Value, RowIndex);

	AddSection(*NewSection);

	return NewSection;
}

#undef LOCTEXT_NAMESPACE
