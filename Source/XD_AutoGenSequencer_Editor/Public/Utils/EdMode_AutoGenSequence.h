// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"

class ISequencer;

/**
 *
 */
class XD_AUTOGENSEQUENCER_EDITOR_API FEdMode_AutoGenSequence : public FEdMode
{
public:
	FEdMode_AutoGenSequence();

	bool IsCompatibleWith(FEditorModeID OtherModeID) const override { return true; }
	bool UsesToolkits() const override { return false; }
	void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
	void DrawHUD(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) override;

	TWeakPtr<ISequencer> WeakSequencer;
public:
	static FName ID;
};
