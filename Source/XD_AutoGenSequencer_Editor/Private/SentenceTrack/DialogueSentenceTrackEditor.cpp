// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueSentenceTrackEditor.h"
#include "DialogueSentenceTrack.h"
#include "MultiBoxBuilder.h"
#include "ISequencer.h"
#include "GameFramework/Actor.h"
#include "SlateApplication.h"
#include "ScopedTransaction.h"
#include "SequencerSectionPainter.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencerModule"

void FDialogueSentenceTrackEditor::AddKey(const FGuid& ObjectGuid)
{

}

void FDialogueSentenceTrackEditor::BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const TArray<FGuid>& ObjectBindings, const UClass* ObjectClass)
{
	if (ObjectClass->IsChildOf(AActor::StaticClass()))
	{
		const TSharedPtr<ISequencer> ParentSequencer = GetSequencer();

		MenuBuilder.AddMenuEntry(
			LOCTEXT("AddSentence", "说话"), LOCTEXT("AddSentenceTooltip", "Adds an dialogue sentence track."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([=]()
				{
					FSlateApplication::Get().DismissAllMenus();

					TSharedPtr<ISequencer> SequencerPtr = GetSequencer();

					const FScopedTransaction Transaction(LOCTEXT("AddSentence_Transaction", "Add Sentence"));
					for (FGuid ObjectBinding : ObjectBindings)
					{
						UObject* Object = SequencerPtr->FindSpawnedObjectOrTemplate(ObjectBinding);
						int32 RowIndex = INDEX_NONE;
						AnimatablePropertyChanged(FOnKeyProperty::CreateLambda([=](FFrameNumber KeyTime)
							{
								FKeyPropertyResult KeyPropertyResult;
								if (UObject* Object = SequencerPtr->FindSpawnedObjectOrTemplate(ObjectBinding))
								{
									FFindOrCreateHandleResult HandleResult = FindOrCreateHandleToObject(Object);
									FGuid ObjectHandle = HandleResult.Handle;
									KeyPropertyResult.bHandleCreated |= HandleResult.bWasCreated;

									if (ObjectHandle.IsValid())
									{
										UMovieSceneTrack* Track = nullptr;
										if (!Track)
										{
											Track = AddTrack(GetSequencer()->GetFocusedMovieSceneSequence()->GetMovieScene(), ObjectHandle, UDialogueSentenceTrack::StaticClass(), NAME_None);
											KeyPropertyResult.bTrackCreated = true;
										}

// 										if (ensure(Track))
// 										{
// 											Track->Modify();
// 
// 											UMovieSceneSection* NewSection = Cast<UDialogueSentenceTrack>(Track)->AddNewAnimationOnRow(KeyTime, AnimSequence, RowIndex);
// 											KeyPropertyResult.bTrackModified = true;
// 
// 											GetSequencer()->EmptySelection();
// 											GetSequencer()->SelectSection(NewSection);
// 											GetSequencer()->ThrobSectionSelection();
// 										}
									}

								}
								return KeyPropertyResult;
							}));
					}
				})
			)
		);
	}
}

bool FDialogueSentenceTrackEditor::HandleAssetAdded(UObject* Asset, const FGuid& TargetObjectGuid)
{
	return false;
}

TSharedRef<ISequencerSection> FDialogueSentenceTrackEditor::MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding)
{
	return MakeShareable(new FDialogueSentenceSection(SectionObject, GetSequencer()));
}

bool FDialogueSentenceTrackEditor::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	return Type == UDialogueSentenceTrack::StaticClass();
}

void FDialogueSentenceTrackEditor::BuildTrackContextMenu(FMenuBuilder& MenuBuilder, UMovieSceneTrack* Track)
{

}

TSharedPtr<SWidget> FDialogueSentenceTrackEditor::BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params)
{
	return nullptr;
}

bool FDialogueSentenceTrackEditor::OnAllowDrop(const FDragDropEvent& DragDropEvent, UMovieSceneTrack* Track, int32 RowIndex, const FGuid& TargetObjectGuid)
{
	return false;
}

FReply FDialogueSentenceTrackEditor::OnDrop(const FDragDropEvent& DragDropEvent, UMovieSceneTrack* Track, int32 RowIndex, const FGuid& TargetObjectGuid)
{
	return FReply::Unhandled();
}

FDialogueSentenceSection::FDialogueSentenceSection(UMovieSceneSection& InSection, TWeakPtr<ISequencer> InSequencer)
	:Section(InSection), 
	Sequencer(InSequencer)
{

}

UMovieSceneSection* FDialogueSentenceSection::GetSectionObject()
{
	return &Section;
}

int32 FDialogueSentenceSection::OnPaintSection(FSequencerSectionPainter& Painter) const
{
	int32 LayerId = Painter.PaintSectionBackground();

	return LayerId;
}

#undef LOCTEXT_NAMESPACE
