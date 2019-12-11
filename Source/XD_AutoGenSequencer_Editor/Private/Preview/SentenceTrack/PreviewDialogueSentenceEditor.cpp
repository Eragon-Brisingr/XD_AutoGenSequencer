// Fill out your copyright notice in the Description page of Project Settings.


#include "Preview/SentenceTrack/PreviewDialogueSentenceEditor.h"
#include <SequencerSectionPainter.h>

#include "Preview/SentenceTrack/PreviewDialogueSentenceTrack.h"
#include "Preview/Sequence/PreviewDialogueSoundSequence.h"
#include "Preview/SentenceTrack/PreviewDialogueSentenceSection.h"
#include "Data/DialogueSentence.h"

FPreviewDialogueSentenceEditor::FPreviewDialogueSentenceEditor(TSharedRef<ISequencer> InSequencer) 
	: FMovieSceneTrackEditor(InSequencer)
{

}

void FPreviewDialogueSentenceEditor::AddKey(const FGuid& ObjectGuid)
{

}

void FPreviewDialogueSentenceEditor::BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const TArray<FGuid>& ObjectBindings, const UClass* ObjectClass)
{

}

bool FPreviewDialogueSentenceEditor::HandleAssetAdded(UObject* Asset, const FGuid& TargetObjectGuid)
{
	return false;
}

TSharedRef<ISequencerSection> FPreviewDialogueSentenceEditor::MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding)
{
	return MakeShareable(new FPreviewDialogueSentenceSection(SectionObject, GetSequencer()));
}

bool FPreviewDialogueSentenceEditor::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	return Type == UPreviewDialogueSentenceTrack::StaticClass();
}

bool FPreviewDialogueSentenceEditor::SupportsSequence(UMovieSceneSequence* InSequence) const
{
	return InSequence->GetClass() == UPreviewDialogueSoundSequence::StaticClass();
}

void FPreviewDialogueSentenceEditor::BuildTrackContextMenu(FMenuBuilder& MenuBuilder, UMovieSceneTrack* Track)
{

}

TSharedPtr<SWidget> FPreviewDialogueSentenceEditor::BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params)
{
	return nullptr;
}

bool FPreviewDialogueSentenceEditor::OnAllowDrop(const FDragDropEvent& DragDropEvent, UMovieSceneTrack* Track, int32 RowIndex, const FGuid& TargetObjectGuid)
{
	return false;
}

FReply FPreviewDialogueSentenceEditor::OnDrop(const FDragDropEvent& DragDropEvent, UMovieSceneTrack* Track, int32 RowIndex, const FGuid& TargetObjectGuid)
{
	return FReply::Handled();
}

FPreviewDialogueSentenceSection::FPreviewDialogueSentenceSection(UMovieSceneSection& InSection, TWeakPtr<ISequencer> InSequencer)
	:Section(*Cast<UPreviewDialogueSentenceSection>(&InSection)),
	Sequencer(InSequencer)
{

}

UMovieSceneSection* FPreviewDialogueSentenceSection::GetSectionObject()
{
	return &Section;
}

FText FPreviewDialogueSentenceSection::GetSectionTitle() const
{
	if (UDialogueSentence* DialogueSentence = Section.DialogueSentenceEditData.DialogueSentence)
	{
		return DialogueSentence->GetSubTitle();
	}
	return FText::GetEmpty();
}

float FPreviewDialogueSentenceSection::GetSectionHeight() const
{
	return Section.GetTypedOuter<UPreviewDialogueSentenceTrack>()->GetRowHeight();
}

int32 FPreviewDialogueSentenceSection::OnPaintSection(FSequencerSectionPainter& Painter) const
{
	int32 LayerId = Painter.PaintSectionBackground();

	return LayerId;
}

void FPreviewDialogueSentenceSection::BeginResizeSection()
{
	UPreviewDialogueSentenceSection* DialogueSentenceSection = &Section;
	InitialStartOffsetDuringResize = DialogueSentenceSection->GetStartOffset();
	InitialStartTimeDuringResize = DialogueSentenceSection->HasStartFrame() ? DialogueSentenceSection->GetInclusiveStartFrame() : 0;
}

void FPreviewDialogueSentenceSection::ResizeSection(ESequencerSectionResizeMode ResizeMode, FFrameNumber ResizeTime)
{
	UPreviewDialogueSentenceSection* DialogueSentenceSection = &Section;

	if (ResizeMode == SSRM_LeadingEdge && DialogueSentenceSection)
	{
		FFrameNumber NewStartOffset = ResizeTime - InitialStartTimeDuringResize;
		NewStartOffset += InitialStartOffsetDuringResize;

		// Ensure start offset is not less than 0
		if (NewStartOffset < 0)
		{
			ResizeTime = ResizeTime - NewStartOffset;
			NewStartOffset = FFrameNumber(0);
		}

		DialogueSentenceSection->SetStartOffset(NewStartOffset);
	}

	ISequencerSection::ResizeSection(ResizeMode, ResizeTime);
}

void FPreviewDialogueSentenceSection::BeginSlipSection()
{
	UPreviewDialogueSentenceSection* DialogueSentenceSection = &Section;
	InitialStartOffsetDuringResize = DialogueSentenceSection->GetStartOffset();
	InitialStartTimeDuringResize = DialogueSentenceSection->HasStartFrame() ? DialogueSentenceSection->GetInclusiveStartFrame() : 0;
}

void FPreviewDialogueSentenceSection::SlipSection(FFrameNumber SlipTime)
{
	UPreviewDialogueSentenceSection* DialogueSentenceSection = &Section;

	FFrameNumber NewStartOffset = SlipTime - InitialStartTimeDuringResize;
	NewStartOffset += InitialStartOffsetDuringResize;

	// Ensure start offset is not less than 0
	DialogueSentenceSection->SetStartOffset(FMath::Max(NewStartOffset, FFrameNumber(0)));

	ISequencerSection::SlipSection(SlipTime);
}

