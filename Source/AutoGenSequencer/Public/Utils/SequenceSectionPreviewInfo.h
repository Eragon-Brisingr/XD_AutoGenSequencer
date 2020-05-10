// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SequenceSectionPreviewInfo.generated.h"

class FViewport;
class FSceneView;
class FCanvas;
class IMovieScenePlayer;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USequenceSectionPreviewInfo : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class AUTOGENSEQUENCER_API ISequenceSectionPreviewInfo
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
#if WITH_EDITOR
	// 当Section被选中时绘制预览信息
	virtual void DrawSectionSelectedPreviewInfo(IMovieScenePlayer* Player, const FFrameNumber& FramePosition, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) const = 0;
	// 当Section在执行中时绘制预览信息
	virtual void DrawSectionExecutePreviewInfo(IMovieScenePlayer* Player, const FFrameNumber& FramePosition, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) const = 0;
#endif
};
