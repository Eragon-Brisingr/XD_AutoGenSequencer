// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include <Modules/ModuleManager.h>
#include "ISettingsSection.h"

class UPreviewDialogueSoundSequence;
class FAssetTypeActions_DialogueSentence;
class FAssetTypeActions_AutoGenDialogueAnimSet;
class FAssetTypeActions_AutoGenDialogueCameraSet;
class FAssetTypeActions_AutoGenDialogueCharacterSettings;
class FAssetTypeActions_DialogueStandPositionTemplate;
class FAssetTypeActions_AutoGenDialogueCameraTemplate;

class FAutoGenSequencer_EditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	void StartupModule() override;
	void ShutdownModule() override;

	static uint32 AutoGenDialogueSequence_AssetCategory;
private:
	FDelegateHandle DialogueSentenceTrackEditorHandle;
	FDelegateHandle CameraTrackingTrackEditorHandle;

	FName DialogueCharacterDataTypeName = TEXT("DialogueCharacterData");
	FName DialogueSentenceEditDataTypeName = TEXT("DialogueSentenceEditData");
	FName DialogueCharacterName = TEXT("DialogueCharacterName");

	ISettingsSectionPtr RuntimeSettingsSection;
	ISettingsSectionPtr SettingsSection;

	TSharedPtr<FAssetTypeActions_DialogueSentence> AssetTypeActions_DialogueSentence;
	TSharedPtr<FAssetTypeActions_DialogueStandPositionTemplate> AssetTypeActions_DialogueStandPositionTemplate;
	TSharedPtr<FAssetTypeActions_AutoGenDialogueCameraTemplate> AssetTypeActions_AutoGenDialogueCameraTemplate;
	TSharedPtr<FAssetTypeActions_AutoGenDialogueAnimSet> AssetTypeActions_AutoGenDialogueAnimSet;
	TSharedPtr<FAssetTypeActions_AutoGenDialogueCameraSet> AssetTypeActions_AutoGenDialogueCameraSet;
	TSharedPtr<FAssetTypeActions_AutoGenDialogueCharacterSettings> AssetTypeActions_AutoGenDialogueCharacterSettings;
};
