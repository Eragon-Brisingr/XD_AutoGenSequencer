// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/EdMode_AutoGenSequence.h"
#include <CanvasTypes.h>
#include <CanvasItem.h>
#include <ISequencer.h>
#include <MovieSceneSection.h>
#include <MovieSceneSequence.h>
#include <MovieScene.h>
#include <Tracks/MovieSceneSpawnTrack.h>
#include <Sections/MovieSceneBoolSection.h>
#include <Editor.h>
#include <LevelEditorViewport.h>
#include <GameFramework/Character.h>

#include "Utils/SequenceSectionPreviewInfo.h"
#include "Utils/GenDialogueSequenceEditor.h"
#include "Utils/DialogueCameraUtils.h"

FName FEdMode_AutoGenSequence::ID = TEXT("EdMode_AutoGenSequence");

FEdMode_AutoGenSequence::FEdMode_AutoGenSequence()
{
}

void FEdMode_AutoGenSequence::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{

}

void FEdMode_AutoGenSequence::DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
	FGenDialogueSequenceEditor& GenDialogueSequenceEditor = FGenDialogueSequenceEditor::Get();
	if (GenDialogueSequenceEditor.WeakSequencer.IsValid())
	{
		ISequencer* Sequencer = GenDialogueSequenceEditor.WeakSequencer.Pin().Get();

		// 只有在镜头被激活时才进行绘制
		if (!Sequencer->IsPerspectiveViewportCameraCutEnabled())
		{
			return;
		}

		UMovieSceneSequence* MovieSceneSequence = Sequencer->GetFocusedMovieSceneSequence();
		UMovieScene* MovieScene = MovieSceneSequence->GetMovieScene();
		const FFrameNumber FramePosition = Sequencer->GetLocalTime().ConvertTo(MovieScene->GetTickResolution()).GetFrame();

		TArray<UMovieSceneSection*> SelectedSections;
		Sequencer->GetSelectedSections(SelectedSections);
		for (UMovieSceneSection* Section : SelectedSections)
		{
			if (ISequenceSectionPreviewInfo* PreviewInfo = Cast<ISequenceSectionPreviewInfo>(Section))
			{
				PreviewInfo->DrawSectionSelectedPreviewInfo(Sequencer, FramePosition, Viewport, View, Canvas);
			}
		}

		for (const FMovieSceneBinding& Binding : MovieScene->GetBindings())
		{
			// 考虑组件的Binding，组件需要找到Owner的MovieSceneSpawnTrack
			FGuid ParentGuid = Binding.GetObjectGuid();
			for (const FMovieScenePossessable* MovieScenePossessable = MovieScene->FindPossessable(ParentGuid); MovieScenePossessable;
				MovieScenePossessable = MovieScene->FindPossessable(ParentGuid))
			{
				if (MovieScenePossessable->GetParent().IsValid())
				{
					ParentGuid = MovieScenePossessable->GetParent();
				}
				else
				{
					break;
				}
			}

			bool IsSpawned = true;
			if (UMovieSceneSpawnTrack* SpawnTrack = MovieScene->FindTrack<UMovieSceneSpawnTrack>(ParentGuid))
			{
				UMovieSceneBoolSection* SpawnSection = Cast<UMovieSceneBoolSection>(SpawnTrack->GetAllSections()[0]);
				SpawnSection->GetChannel().Evaluate(FramePosition, IsSpawned);
			}
			if (IsSpawned)
			{
				for (UMovieSceneTrack* Track : Binding.GetTracks())
				{
					for (UMovieSceneSection* Section : Track->GetAllSections())
					{
						if (Section->IsActive() && Section->IsTimeWithinSection(FramePosition))
						{
							if (ISequenceSectionPreviewInfo* PreviewInfo = Cast<ISequenceSectionPreviewInfo>(Section))
							{
								PreviewInfo->DrawSectionExecutePreviewInfo(Sequencer, FramePosition, Viewport, View, Canvas);
							}
						}
					}
				}
			}
		}

		const FIntRect CanvasRect = Canvas->GetViewRect();
		for (const TPair<FName, TSoftObjectPtr<ACharacter>>& Pair : GenDialogueSequenceEditor.CharacterNameInstanceMap)
		{
			if (const ACharacter* Character = Pair.Value.Get())
			{
				FBox2D CharacterScreenBounds;
				FVector Origin, BoxExtent;
				Character->GetActorBounds(true, Origin, BoxExtent);
				if (FDialogueCameraUtils::ProjectWorldBoxBoundsToScreen(View, CanvasRect, Origin, BoxExtent, CharacterScreenBounds))
				{
					CharacterScreenBounds = FDialogueCameraUtils::ClampBoundsInScreenRect(CharacterScreenBounds, CanvasRect);
					FCanvasBoxItem CanvasBoxItem(CharacterScreenBounds.GetCenter() - CharacterScreenBounds.GetExtent(), CharacterScreenBounds.GetSize());
					CanvasBoxItem.LineThickness = 2.f;
					CanvasBoxItem.SetColor(FColor::Green);
					Canvas->DrawItem(CanvasBoxItem);
				}
			}
		}
	}
}
