// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include <Modules/ModuleManager.h>
#include "ISettingsSection.h"

class UPreviewDialogueSoundSequence;
class FAssetTypeActions_DialogueSentence;
class FAssetTypeActions_AutoGenDialogueAnimSet;
class FAssetTypeActions_AutoGenDialogueCameraSet;

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

	FName DialogueCharacterDataTypeName = TEXT("DialogueCharacterData");
	FName DialogueSentenceEditDataTypeName = TEXT("DialogueSentenceEditData");
	FName DialogueCharacterName = TEXT("DialogueCharacterName");

	ISettingsSectionPtr RuntimeSettingsSection;
	ISettingsSectionPtr SettingsSection;

	TSharedPtr<FAssetTypeActions_DialogueSentence> AssetTypeActions_DialogueSentence;
	TSharedPtr<FAssetTypeActions_AutoGenDialogueAnimSet> AssetTypeActions_AutoGenDialogueAnimSet;
	TSharedPtr<FAssetTypeActions_AutoGenDialogueCameraSet> AssetTypeActions_AutoGenDialogueCameraSet;
};
