// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class UPreviewDialogueSoundSequence;

class FXD_AutoGenSequencer_EditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	FDelegateHandle DialogueSentenceTrackEditorHandle;
	FDelegateHandle PreviewDialogueSentenceTrackEditorHandle;

	FName DialogueStationInstanceOverrideTypeName = TEXT("DialogueStationInstanceOverride");
	FName DialogueSentenceEditDataTypeName = TEXT("DialogueSentenceEditData");
};
