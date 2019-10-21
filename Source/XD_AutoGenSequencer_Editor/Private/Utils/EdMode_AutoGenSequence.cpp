// Fill out your copyright notice in the Description page of Project Settings.


#include "EdMode_AutoGenSequence.h"
#include "CanvasTypes.h"
#include "CanvasItem.h"
#include "ISequencer.h"
#include "MovieSceneSection.h"
#include "XD_SequenceSectionPreviewInfo.h"
#include "MovieSceneSequence.h"
#include "MovieScene.h"
#include "MovieSceneSpawnTrack.h"
#include "MovieSceneBoolSection.h"
#include "Editor.h"
#include "LevelEditorViewport.h"

FName FEdMode_AutoGenSequence::ID = TEXT("EdMode_AutoGenSequence");

FEdMode_AutoGenSequence::FEdMode_AutoGenSequence()
{
}

void FEdMode_AutoGenSequence::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{

}

void FEdMode_AutoGenSequence::DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas)
{
	if (WeakSequencer.IsValid())
	{
		ISequencer* Sequencer = WeakSequencer.Pin().Get();

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
			if (IXD_SequenceSectionPreviewInfo* PreviewInfo = Cast<IXD_SequenceSectionPreviewInfo>(Section))
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
							if (IXD_SequenceSectionPreviewInfo* PreviewInfo = Cast<IXD_SequenceSectionPreviewInfo>(Section))
							{
								PreviewInfo->DrawSectionExecutePreviewInfo(Sequencer, FramePosition, Viewport, View, Canvas);
							}
						}
					}
				}
			}
		}
	}
}
