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

	static uint32 AutoGenDialogueSequence_AssetCategory;
	FName AutoGenDialogueSequence_AssetCategoryKey = TEXT("XD_AutoGenSequencer");
private:
	FDelegateHandle DialogueSentenceTrackEditorHandle;
	FDelegateHandle PreviewDialogueSentenceTrackEditorHandle;
	FDelegateHandle TwoTargetCameraTrackingTrackEditorHandle;

	FName DialogueStationInstanceOverrideTypeName = TEXT("DialogueStationInstanceOverride");
	FName DialogueSentenceEditDataTypeName = TEXT("DialogueSentenceEditData");
	FName DialogueCharacterName = TEXT("DialogueCharacterName");
};
