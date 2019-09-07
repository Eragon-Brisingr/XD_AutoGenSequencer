// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "XD_AutoGenSequencer_Editor.h"
#include "ISequencerModule.h"
#include "AutoGenSequencerCBExtensions.h"
#include "DialogueSentenceTrackEditor.h"
#include "PreviewDialogueSentenceEditor.h"
#include "DialogueSequenceExtender.h"
#include "PropertyEditorModule.h"
#include "DialogueSentenceCustomization.h"
#include "TwoTargetCameraTrackingEditor.h"
#include <ISettingsModule.h>
#include <ISettingsSection.h>
#include "AutoGenDialogueSettings.h"
#include "AssetToolsModule.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

uint32 FXD_AutoGenSequencer_EditorModule::AutoGenDialogueSequence_AssetCategory;

void FXD_AutoGenSequencer_EditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AutoGenDialogueSequence_AssetCategory = AssetTools.RegisterAdvancedAssetCategory(AutoGenDialogueSequence_AssetCategoryKey, LOCTEXT("自动生成对白系统分类名", "自动生成对白系统"));
	}

	FAutoGenSequencerContentBrowserExtensions::RegisterExtender();

	ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
	{
		DialogueSentenceTrackEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateLambda([](TSharedRef<ISequencer> InSequencer)
			{
				return MakeShareable(new FDialogueSentenceTrackEditor(InSequencer));
			}));
		PreviewDialogueSentenceTrackEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateLambda([](TSharedRef<ISequencer> InSequencer)
			{
				return MakeShareable(new FPreviewDialogueSentenceEditor(InSequencer));
			}));
		TwoTargetCameraTrackingTrackEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateLambda([](TSharedRef<ISequencer> InSequencer)
			{
				return MakeShareable(new FTwoTargetCameraTrackingEditor(InSequencer));
			}));
		FDialogueSequenceExtender::Get().Register(SequencerModule);
	}

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	{
		PropertyModule.RegisterCustomPropertyTypeLayout(DialogueStationInstanceOverrideTypeName, FOnGetPropertyTypeCustomizationInstance::CreateLambda([=]()
			{
				return MakeShareable(new FDialogueStationInstanceOverride_Customization());
			}));
		PropertyModule.RegisterCustomPropertyTypeLayout(DialogueSentenceEditDataTypeName, FOnGetPropertyTypeCustomizationInstance::CreateLambda([=]()
			{
				return MakeShareable(new FDialogueSentenceEditData_Customization());
			}));
		PropertyModule.RegisterCustomPropertyTypeLayout(DialogueCharacterName, FOnGetPropertyTypeCustomizationInstance::CreateLambda([=]()
			{
				return MakeShareable(new FDialogueCharacterName_Customization());
			}));
	}

	// register settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "XD_AutoGenSequencer",
			LOCTEXT("AutoGenDialogueSettings", "AutoGenDialogueSettings"),
			LOCTEXT("AutoGenDialogueSettingsDescription", "Configure the AutoGenDialogueSettings plug-in."),
			GetMutableDefault<UAutoGenDialogueSettings>()
		);
	}
}

void FXD_AutoGenSequencer_EditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AutoGenDialogueSequence_AssetCategory = AssetTools.RegisterAdvancedAssetCategory(AutoGenDialogueSequence_AssetCategoryKey, LOCTEXT("自动生成对白系统分类名", "自动生成对白系统"));
	}

	FAutoGenSequencerContentBrowserExtensions::UnregisterExtender();

	if (FModuleManager::Get().IsModuleLoaded("Sequencer"))
	{
		ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
		SequencerModule.UnRegisterTrackEditor(DialogueSentenceTrackEditorHandle);
		SequencerModule.UnRegisterTrackEditor(PreviewDialogueSentenceTrackEditorHandle);
		SequencerModule.UnRegisterTrackEditor(TwoTargetCameraTrackingTrackEditorHandle);
		FDialogueSequenceExtender::Get().Unregister(SequencerModule);
	}

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		SequencerModule.UnregisterCustomPropertyTypeLayout(DialogueStationInstanceOverrideTypeName);
		SequencerModule.UnregisterCustomPropertyTypeLayout(DialogueSentenceEditDataTypeName);
		SequencerModule.UnregisterCustomPropertyTypeLayout(DialogueCharacterName);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FXD_AutoGenSequencer_EditorModule, XD_AutoGenSequencer_Editor)