// Fill out your copyright notice in the Description page of Project Settings.


#include "TrackEditors/DialogueSentenceTrackEditor.h"
#include <Framework/MultiBox/MultiBoxBuilder.h>
#include <ISequencer.h>
#include <GameFramework/Actor.h>
#include <Framework/Application/SlateApplication.h>
#include <ScopedTransaction.h>
#include <SequencerSectionPainter.h>
#include <LevelSequence.h>
#include <SequencerUtilities.h>
#include <AssetRegistryModule.h>
#include <Sound/SoundBase.h>
#include <ContentBrowserModule.h>
#include <Widgets/Layout/SBox.h>
#include <IContentBrowserSingleton.h>
#include <Widgets/Input/SButton.h>

#include "Tracks/SentenceTrack/DialogueSentenceTrack.h"
#include "Tracks/SentenceTrack/DialogueSentenceSection.h"
#include "Data/DialogueSentence.h"
#include "Interface/XD_AutoGenDialogueInterface.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencerModule"

void FDialogueSentenceTrackEditor::AddKey(const FGuid& ObjectGuid)
{

}

void FDialogueSentenceTrackEditor::BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const TArray<FGuid>& ObjectBindings, const UClass* ObjectClass)
{
	if (ObjectClass->ImplementsInterface(UXD_AutoGenDialogueInterface::StaticClass()))
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
	return MakeShareable(new FDialogueSentenceSectionEditor(SectionObject, GetSequencer()));
}

bool FDialogueSentenceTrackEditor::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	return Type == UDialogueSentenceTrack::StaticClass();
}

bool FDialogueSentenceTrackEditor::SupportsSequence(UMovieSceneSequence* InSequence) const
{
	return InSequence->GetClass()->IsChildOf(ULevelSequence::StaticClass());
}

void FDialogueSentenceTrackEditor::BuildTrackContextMenu(FMenuBuilder& MenuBuilder, UMovieSceneTrack* Track)
{

}

TSharedPtr<SWidget> FDialogueSentenceTrackEditor::BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params)
{
	// Create a container edit box
	return SNew(SHorizontalBox)

		// Add the audio combo box
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			FSequencerUtilities::MakeAddButton(LOCTEXT("DialogueSentenceText", "Dialogue Sentence"), FOnGetContent::CreateLambda([=]()
				{
					FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
					TArray<FName> ClassNames;
					ClassNames.Add(UDialogueSentence::StaticClass()->GetFName());
					TSet<FName> DerivedClassNames;
					AssetRegistryModule.Get().GetDerivedClassNames(ClassNames, TSet<FName>(), DerivedClassNames);

					FMenuBuilder MenuBuilder(true, nullptr);

					FAssetPickerConfig AssetPickerConfig;
					{
 						AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateRaw(this, &FDialogueSentenceTrackEditor::OnDialogueWaveAssetSelected, Track);
 						AssetPickerConfig.OnAssetEnterPressed = FOnAssetEnterPressed::CreateRaw(this, &FDialogueSentenceTrackEditor::OnDialogueWaveAssetEnterPressed, Track);
						AssetPickerConfig.bAllowNullSelection = false;
						AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;

						for (auto ClassName : DerivedClassNames)
						{
							AssetPickerConfig.Filter.ClassNames.Add(ClassName);
						}
					}

					FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

					TSharedPtr<SBox> MenuEntry = SNew(SBox)
						.WidthOverride(300.0f)
						.HeightOverride(300.f)
						[
							ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
						];

					MenuBuilder.AddWidget(MenuEntry.ToSharedRef(), FText::GetEmpty(), true);

					return MenuBuilder.MakeWidget();
				}), Params.NodeIsHovered, GetSequencer())
		];
}

bool FDialogueSentenceTrackEditor::OnAllowDrop(const FDragDropEvent& DragDropEvent, UMovieSceneTrack* Track, int32 RowIndex, const FGuid& TargetObjectGuid)
{
	return false;
}

FReply FDialogueSentenceTrackEditor::OnDrop(const FDragDropEvent& DragDropEvent, UMovieSceneTrack* Track, int32 RowIndex, const FGuid& TargetObjectGuid)
{
	return FReply::Unhandled();
}

void FDialogueSentenceTrackEditor::OnDialogueWaveAssetSelected(const FAssetData& AssetData, UMovieSceneTrack* Track)
{
	FSlateApplication::Get().DismissAllMenus();

	UObject* SelectedObject = AssetData.GetAsset();

	if (SelectedObject)
	{
		UDialogueSentence* NewDialogueSentence = CastChecked<UDialogueSentence>(AssetData.GetAsset());
		if (NewDialogueSentence != nullptr)
		{
			const FScopedTransaction Transaction(NSLOCTEXT("Sequencer", "AddAudio_Transaction", "Add Audio"));

			auto AudioTrack = Cast<UDialogueSentenceTrack>(Track);
			AudioTrack->Modify();

			FFrameTime KeyTime = GetSequencer()->GetLocalTime().Time;
			UMovieSceneSection* NewSection = AudioTrack->AddNewSentenceOnRow(NewDialogueSentence, KeyTime.FrameNumber);

			GetSequencer()->EmptySelection();
			GetSequencer()->SelectSection(NewSection);
			GetSequencer()->ThrobSectionSelection();

			GetSequencer()->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
		}
	}
}

void FDialogueSentenceTrackEditor::OnDialogueWaveAssetEnterPressed(const TArray<FAssetData>& AssetData, UMovieSceneTrack* Track)
{
	if (AssetData.Num() > 0)
	{
		OnDialogueWaveAssetSelected(AssetData[0].GetAsset(), Track);
	}
}

FDialogueSentenceSectionEditor::FDialogueSentenceSectionEditor(UMovieSceneSection& InSection, TWeakPtr<ISequencer> InSequencer)
	:Section(*Cast<UDialogueSentenceSection>(&InSection)),
	Sequencer(InSequencer)
{

}

UMovieSceneSection* FDialogueSentenceSectionEditor::GetSectionObject()
{
	return &Section;
}

FText FDialogueSentenceSectionEditor::GetSectionTitle() const
{
	if (Section.DialogueSentence)
	{
		return Section.DialogueSentence->GetSubTitle();
	}
	return LOCTEXT("NoDialogueSentenceTitleName", "No Dialogue Sentence");
}

float FDialogueSentenceSectionEditor::GetSectionHeight() const
{
	return Section.GetTypedOuter<UDialogueSentenceTrack>()->GetRowHeight();
}

int32 FDialogueSentenceSectionEditor::OnPaintSection(FSequencerSectionPainter& Painter) const
{
	int32 LayerId = Painter.PaintSectionBackground();

	return LayerId;
}

void FDialogueSentenceSectionEditor::BeginResizeSection()
{
	UDialogueSentenceSection* DialogueSentenceSection = &Section;
	InitialStartOffsetDuringResize = DialogueSentenceSection->StartFrameOffset;
	InitialStartTimeDuringResize = DialogueSentenceSection->HasStartFrame() ? DialogueSentenceSection->GetInclusiveStartFrame() : 0;
}

void FDialogueSentenceSectionEditor::ResizeSection(ESequencerSectionResizeMode ResizeMode, FFrameNumber ResizeTime)
{
	UDialogueSentenceSection* DialogueSentenceSection = &Section;

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

		DialogueSentenceSection->StartFrameOffset = NewStartOffset;
	}

	ISequencerSection::ResizeSection(ResizeMode, ResizeTime);
}

void FDialogueSentenceSectionEditor::BeginSlipSection()
{
	UDialogueSentenceSection* DialogueSentenceSection = &Section;
	InitialStartOffsetDuringResize = DialogueSentenceSection->StartFrameOffset;
	InitialStartTimeDuringResize = DialogueSentenceSection->HasStartFrame() ? DialogueSentenceSection->GetInclusiveStartFrame() : 0;
}

void FDialogueSentenceSectionEditor::SlipSection(FFrameNumber SlipTime)
{
	UDialogueSentenceSection* DialogueSentenceSection = &Section;

	FFrameNumber NewStartOffset = SlipTime - InitialStartTimeDuringResize;
	NewStartOffset += InitialStartOffsetDuringResize;

	// Ensure start offset is not less than 0
	DialogueSentenceSection->StartFrameOffset = FMath::Max(NewStartOffset, FFrameNumber(0));

	ISequencerSection::SlipSection(SlipTime);
}

#undef LOCTEXT_NAMESPACE
