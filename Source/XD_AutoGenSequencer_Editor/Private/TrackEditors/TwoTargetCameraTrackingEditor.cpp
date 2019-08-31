// Fill out your copyright notice in the Description page of Project Settings.


#include "TwoTargetCameraTrackingEditor.h"
#include "TwoTargetCameraTrackingTrack.h"
#include "TwoTargetCameraTrackingSection.h"

#include "LevelSequence.h"
#include "SequencerSectionPainter.h"
#include "MultiBoxBuilder.h"
#include "SlateApplication.h"
#include "CineCameraComponent.h"
#include "SequencerUtilities.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencerModule"

void FTwoTargetCameraTrackingEditor::BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const TArray<FGuid>& ObjectBindings, const UClass* ObjectClass)
{
	if (ObjectClass->IsChildOf<UCineCameraComponent>())
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("添加双目标追踪导轨标题", "双目标追踪"), LOCTEXT("添加双目标追踪导轨提示", "添加双目标追踪导轨"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([=]()
				{
					FSlateApplication::Get().DismissAllMenus();

					TSharedPtr<ISequencer> SequencerPtr = GetSequencer();

					const FScopedTransaction Transaction(LOCTEXT("添加双目标追踪导轨", "添加双目标追踪导轨"));
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
											Track = AddTrack(GetSequencer()->GetFocusedMovieSceneSequence()->GetMovieScene(), ObjectHandle, UTwoTargetCameraTrackingTrack::StaticClass(), NAME_None);
											Track->AddSection(*Track->CreateNewSection());
											KeyPropertyResult.bTrackCreated = true;
										}
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

TSharedPtr<SWidget> FTwoTargetCameraTrackingEditor::BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params)
{
	return nullptr;
}

bool FTwoTargetCameraTrackingEditor::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	return Type == UTwoTargetCameraTrackingTrack::StaticClass();
}

bool FTwoTargetCameraTrackingEditor::SupportsSequence(UMovieSceneSequence* InSequence) const
{
	return InSequence->GetClass()->IsChildOf(ULevelSequence::StaticClass());
}

FTwoTargetCameraTrackingSectionEditor::FTwoTargetCameraTrackingSectionEditor(UMovieSceneSection& InSection, TWeakPtr<ISequencer> InSequencer) 
	:Section(*Cast<UTwoTargetCameraTrackingSection>(&InSection)), Sequencer(InSequencer)
{

}

UMovieSceneSection* FTwoTargetCameraTrackingSectionEditor::GetSectionObject()
{
	return &Section;
}

FText FTwoTargetCameraTrackingSectionEditor::GetSectionTitle() const
{
	return LOCTEXT("NoTargetTitleName", "No Target");
}

float FTwoTargetCameraTrackingSectionEditor::GetSectionHeight() const
{
	return 18.f;
}

int32 FTwoTargetCameraTrackingSectionEditor::OnPaintSection(FSequencerSectionPainter& Painter) const
{
	int32 LayerId = Painter.PaintSectionBackground();

	return LayerId;
}

void FTwoTargetCameraTrackingSectionEditor::BeginResizeSection()
{

}

void FTwoTargetCameraTrackingSectionEditor::ResizeSection(ESequencerSectionResizeMode ResizeMode, FFrameNumber ResizeTime)
{

}

void FTwoTargetCameraTrackingSectionEditor::BeginSlipSection()
{

}

void FTwoTargetCameraTrackingSectionEditor::SlipSection(FFrameNumber SlipTime)
{
	ISequencerSection::SlipSection(SlipTime);
}

#undef LOCTEXT_NAMESPACE
