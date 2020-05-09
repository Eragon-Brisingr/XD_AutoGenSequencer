// Fill out your copyright notice in the Description page of Project Settings.


#include "TrackEditors/CameraTrackingEditor.h"
#include <LevelSequence.h>
#include <SequencerSectionPainter.h>
#include <Framework/MultiBox/MultiBoxBuilder.h>
#include <Framework/Application/SlateApplication.h>
#include <CineCameraComponent.h>
#include <SequencerUtilities.h>
#include <UObject/UObjectHash.h>

#include "Tracks/CameraTrackingTrack/CameraTrackingTrackBase.h"

#define LOCTEXT_NAMESPACE "FAutoGenSequencerModule"

void FCameraTrackingEditor::BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const TArray<FGuid>& ObjectBindings, const UClass* ObjectClass)
{
	if (ObjectClass->IsChildOf<UCineCameraComponent>())
	{
		TArray<UClass*> DerivedClasses;
		GetDerivedClasses(UCameraTrackingTrackBase::StaticClass(), DerivedClasses, true);
		for (UClass* DerivedClasse : DerivedClasses)
		{
			if (DerivedClasse->HasAnyClassFlags(CLASS_Abstract))
			{
				continue;
			}

			FText Title = FText::Format(LOCTEXT("添加镜头追踪导轨标题", "添加[{0}]"), DerivedClasse->GetDisplayNameText());
			MenuBuilder.AddMenuEntry(
				Title, Title,
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
											UMovieScene* MoiveScene = GetSequencer()->GetFocusedMovieSceneSequence()->GetMovieScene();
											UMovieSceneTrack* ExistedTrack = nullptr;
											for (UClass* TrackClasse : DerivedClasses)
											{
												ExistedTrack = MoiveScene->FindTrack(TrackClasse, ObjectBinding);
												if (ExistedTrack)
												{
													break;
												}
											}
											if (ExistedTrack == nullptr)
											{
												UMovieSceneTrack* Track = AddTrack(GetSequencer()->GetFocusedMovieSceneSequence()->GetMovieScene(), ObjectHandle, DerivedClasse, NAME_None);
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
}

TSharedPtr<SWidget> FCameraTrackingEditor::BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params)
{
	return nullptr;
}

bool FCameraTrackingEditor::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	TArray<UClass*> DerivedClasses;
	GetDerivedClasses(UCameraTrackingTrackBase::StaticClass(), DerivedClasses, true);
	for (UClass* DerivedClasse : DerivedClasses)
	{
		if (Type == DerivedClasse)
		{
			return true;
		}
	}
	return false;
}

bool FCameraTrackingEditor::SupportsSequence(UMovieSceneSequence* InSequence) const
{
	return InSequence->GetClass()->IsChildOf(ULevelSequence::StaticClass());
}

FCameraTrackingSectionEditor::FCameraTrackingSectionEditor(UMovieSceneSection& InSection, TWeakPtr<ISequencer> InSequencer) 
	:Section(InSection), Sequencer(InSequencer)
{

}

UMovieSceneSection* FCameraTrackingSectionEditor::GetSectionObject()
{
	return &Section;
}

FText FCameraTrackingSectionEditor::GetSectionTitle() const
{
	return FText();
}

float FCameraTrackingSectionEditor::GetSectionHeight() const
{
	return 18.f;
}

int32 FCameraTrackingSectionEditor::OnPaintSection(FSequencerSectionPainter& Painter) const
{
	int32 LayerId = Painter.PaintSectionBackground();

	return LayerId;
}

void FCameraTrackingSectionEditor::BeginResizeSection()
{

}

void FCameraTrackingSectionEditor::ResizeSection(ESequencerSectionResizeMode ResizeMode, FFrameNumber ResizeTime)
{

}

void FCameraTrackingSectionEditor::BeginSlipSection()
{

}

void FCameraTrackingSectionEditor::SlipSection(FFrameNumber SlipTime)
{
	ISequencerSection::SlipSection(SlipTime);
}

#undef LOCTEXT_NAMESPACE
