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
#include "AutoGenDialogueRuntimeSettings.h"
#include "ISettingsCategory.h"
#include "DialogueSentenceFactory.h"
#include "AutoGenDialogueAnimSet.h"
#include "AutoGenDialogueCameraSet.h"
#include "EditorModeRegistry.h"
#include "EdMode_AutoGenSequence.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

uint32 FXD_AutoGenSequencer_EditorModule::AutoGenDialogueSequence_AssetCategory;

void FXD_AutoGenSequencer_EditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AutoGenDialogueSequence_AssetCategory = AssetTools.RegisterAdvancedAssetCategory(AutoGenDialogueSequence_AssetCategoryKey, LOCTEXT("自动生成对白系统分类名", "自动生成对白系统"));

		AssetTypeActions_DialogueSentence = MakeShareable(new FAssetTypeActions_DialogueSentence());
		AssetTools.RegisterAssetTypeActions(AssetTypeActions_DialogueSentence.ToSharedRef());

		AssetTypeActions_AutoGenDialogueAnimSet = MakeShareable(new FAssetTypeActions_AutoGenDialogueAnimSet());
		AssetTools.RegisterAssetTypeActions(AssetTypeActions_AutoGenDialogueAnimSet.ToSharedRef());

		AssetTypeActions_AutoGenDialogueCameraSet = MakeShareable(new FAssetTypeActions_AutoGenDialogueCameraSet());
		AssetTools.RegisterAssetTypeActions(AssetTypeActions_AutoGenDialogueCameraSet.ToSharedRef());
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
		PropertyModule.RegisterCustomPropertyTypeLayout(DialogueCharacterDataTypeName, FOnGetPropertyTypeCustomizationInstance::CreateLambda([=]()
			{
				return MakeShareable(new FDialogueCharacterData_Customization());
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
		RuntimeSettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "XD_AutoGenSequencer_Runtime",
			LOCTEXT("AutoGenDialogueRuntimeSettings", "AutoGenDialogueRuntimeSettings"),
			LOCTEXT("AutoGenDialogueRuntimeSettingsDescription", "Configure the AutoGenDialogueRuntimeSettings plug-in."),
			GetMutableDefault<UAutoGenDialogueRuntimeSettings>()
		);
		SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "XD_AutoGenSequencer",
			LOCTEXT("AutoGenDialogueSettings", "AutoGenDialogueSettings"),
			LOCTEXT("AutoGenDialogueSettingsDescription", "Configure the AutoGenDialogueSettings plug-in."),
			GetMutableDefault<UAutoGenDialogueSettings>()
		);
	}

	FEditorModeRegistry::Get().RegisterMode<FEdMode_AutoGenSequence>(FEdMode_AutoGenSequence::ID);
}

void FXD_AutoGenSequencer_EditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (FAssetToolsModule* AssetToolsModule = FModuleManager::Get().GetModulePtr<FAssetToolsModule>("AssetTools"))
	{
		IAssetTools& AssetTools = AssetToolsModule->Get();
		AssetTools.UnregisterAssetTypeActions(AssetTypeActions_DialogueSentence.ToSharedRef());
		AssetTools.UnregisterAssetTypeActions(AssetTypeActions_AutoGenDialogueAnimSet.ToSharedRef());
		AssetTools.UnregisterAssetTypeActions(AssetTypeActions_AutoGenDialogueCameraSet.ToSharedRef());
	}

	FAutoGenSequencerContentBrowserExtensions::UnregisterExtender();

	if (ISequencerModule* SequencerModule = FModuleManager::Get().GetModulePtr<ISequencerModule>("Sequencer"))
	{
		SequencerModule->UnRegisterTrackEditor(DialogueSentenceTrackEditorHandle);
		SequencerModule->UnRegisterTrackEditor(PreviewDialogueSentenceTrackEditorHandle);
		SequencerModule->UnRegisterTrackEditor(TwoTargetCameraTrackingTrackEditorHandle);
		FDialogueSequenceExtender::Get().Unregister(*SequencerModule);
	}

	if (FPropertyEditorModule* SequencerModule = FModuleManager::Get().GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
	{
		SequencerModule->UnregisterCustomPropertyTypeLayout(DialogueCharacterDataTypeName);
		SequencerModule->UnregisterCustomPropertyTypeLayout(DialogueSentenceEditDataTypeName);
		SequencerModule->UnregisterCustomPropertyTypeLayout(DialogueCharacterName);
	}

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		if (RuntimeSettingsSection)
		{
			ISettingsCategory* Category = RuntimeSettingsSection->GetCategory().Pin().Get();
			SettingsModule->UnregisterSettings("Project", Category->GetName(), RuntimeSettingsSection->GetName());
		}
		if (SettingsSection)
		{
			ISettingsCategory* Category = SettingsSection->GetCategory().Pin().Get();
			SettingsModule->UnregisterSettings("Project", Category->GetName(), SettingsSection->GetName());
		}
	}

	FEditorModeRegistry::Get().UnregisterMode(FEdMode_AutoGenSequence::ID);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FXD_AutoGenSequencer_EditorModule, XD_AutoGenSequencer_Editor)