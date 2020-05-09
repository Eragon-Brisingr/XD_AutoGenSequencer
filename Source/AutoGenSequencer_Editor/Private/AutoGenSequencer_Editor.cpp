// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "AutoGenSequencer_Editor.h"
#include <ISequencerModule.h>
#include "AutoGenSequencerCBExtensions.h"
#include "TrackEditors/DialogueSentenceTrackEditor.h"
#include "Utils/GenDialogueSequenceEditor.h"
#include <PropertyEditorModule.h>
#include "Customization/DialogueSentenceCustomization.h"
#include "TrackEditors/CameraTrackingEditor.h"
#include <ISettingsModule.h>
#include <ISettingsSection.h>
#include "Utils/AutoGenDialogueSettings.h"
#include <AssetToolsModule.h>
#include "Utils/AutoGenDialogueRuntimeSettings.h"
#include "ISettingsCategory.h"
#include "Factory/DialogueSentenceFactory.h"
#include "Datas/AutoGenDialogueAnimSet.h"
#include "Datas/AutoGenDialogueCameraSet.h"
#include "EditorModeRegistry.h"
#include "Utils/EdMode_AutoGenSequence.h"
#include "Datas/AutoGenDialogueCharacterSettings.h"
#include "Datas/DialogueStandPositionTemplate.h"
#include "Datas/AutoGenDialogueCameraTemplate.h"

#define LOCTEXT_NAMESPACE "FAutoGenSequencer_EditorModule"

uint32 FAutoGenSequencer_EditorModule::AutoGenDialogueSequence_AssetCategory;

void FAutoGenSequencer_EditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AutoGenDialogueSequence_AssetCategory = AssetTools.RegisterAdvancedAssetCategory(TEXT("自动生成对白系统"), LOCTEXT("自动生成对白系统分类名", "自动生成对白系统"));

		AssetTypeActions_DialogueSentence = MakeShareable(new FAssetTypeActions_DialogueSentence());
		AssetTools.RegisterAssetTypeActions(AssetTypeActions_DialogueSentence.ToSharedRef());

		AssetTypeActions_DialogueStandPositionTemplate = MakeShareable(new FAssetTypeActions_DialogueStandPositionTemplate());
		AssetTools.RegisterAssetTypeActions(AssetTypeActions_DialogueStandPositionTemplate.ToSharedRef());

		AssetTypeActions_AutoGenDialogueCameraTemplate = MakeShareable(new FAssetTypeActions_AutoGenDialogueCameraTemplate());
		AssetTools.RegisterAssetTypeActions(AssetTypeActions_AutoGenDialogueCameraTemplate.ToSharedRef());

		AssetTypeActions_AutoGenDialogueAnimSet = MakeShareable(new FAssetTypeActions_AutoGenDialogueAnimSet());
		AssetTools.RegisterAssetTypeActions(AssetTypeActions_AutoGenDialogueAnimSet.ToSharedRef());

		AssetTypeActions_AutoGenDialogueCameraSet = MakeShareable(new FAssetTypeActions_AutoGenDialogueCameraSet());
		AssetTools.RegisterAssetTypeActions(AssetTypeActions_AutoGenDialogueCameraSet.ToSharedRef());

		AssetTypeActions_AutoGenDialogueCharacterSettings = MakeShareable(new FAssetTypeActions_AutoGenDialogueCharacterSettings());
		AssetTools.RegisterAssetTypeActions(AssetTypeActions_AutoGenDialogueCharacterSettings.ToSharedRef());
	}

	FAutoGenSequencerContentBrowserExtensions::RegisterExtender();

	ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
	{
		DialogueSentenceTrackEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateLambda([](TSharedRef<ISequencer> InSequencer)
			{
				return MakeShareable(new FDialogueSentenceTrackEditor(InSequencer));
			}));
		CameraTrackingTrackEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateLambda([](TSharedRef<ISequencer> InSequencer)
			{
				return MakeShareable(new FCameraTrackingEditor(InSequencer));
			}));
		FGenDialogueSequenceEditor::Get().Register(SequencerModule);
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
		RuntimeSettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "AutoGenSequencer_Runtime",
			LOCTEXT("AutoGenDialogueRuntimeSettings", "AutoGenDialogueRuntimeSettings"),
			LOCTEXT("AutoGenDialogueRuntimeSettingsDescription", "Configure the AutoGenDialogueRuntimeSettings plug-in."),
			GetMutableDefault<UAutoGenDialogueRuntimeSettings>()
		);
		SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "AutoGenSequencer",
			LOCTEXT("AutoGenDialogueSettings", "AutoGenDialogueSettings"),
			LOCTEXT("AutoGenDialogueSettingsDescription", "Configure the AutoGenDialogueSettings plug-in."),
			GetMutableDefault<UAutoGenDialogueSettings>()
		);
	}

	FEditorModeRegistry::Get().RegisterMode<FEdMode_AutoGenSequence>(FEdMode_AutoGenSequence::ID);

	UThumbnailManager& ThumbnailManager = UThumbnailManager::Get();
	{
		ThumbnailManager.RegisterCustomRenderer(UAutoGenDialogueCameraTemplateAsset::StaticClass(), UAutoGenDialogueCameraTemplateThumbnailRenderer::StaticClass());
		ThumbnailManager.RegisterCustomRenderer(UDialogueStandPositionTemplateAsset::StaticClass(), UDialogueStandPositionTemplateThumbnailRenderer::StaticClass());
	}
}

void FAutoGenSequencer_EditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (FAssetToolsModule* AssetToolsModule = FModuleManager::Get().GetModulePtr<FAssetToolsModule>("AssetTools"))
	{
		IAssetTools& AssetTools = AssetToolsModule->Get();
		AssetTools.UnregisterAssetTypeActions(AssetTypeActions_DialogueSentence.ToSharedRef());
		AssetTools.UnregisterAssetTypeActions(AssetTypeActions_DialogueStandPositionTemplate.ToSharedRef());
		AssetTools.UnregisterAssetTypeActions(AssetTypeActions_AutoGenDialogueCameraTemplate.ToSharedRef());
		AssetTools.UnregisterAssetTypeActions(AssetTypeActions_AutoGenDialogueAnimSet.ToSharedRef());
		AssetTools.UnregisterAssetTypeActions(AssetTypeActions_AutoGenDialogueCameraSet.ToSharedRef());
		AssetTools.UnregisterAssetTypeActions(AssetTypeActions_AutoGenDialogueCharacterSettings.ToSharedRef());
	}

	FAutoGenSequencerContentBrowserExtensions::UnregisterExtender();

	if (ISequencerModule* SequencerModule = FModuleManager::Get().GetModulePtr<ISequencerModule>("Sequencer"))
	{
		SequencerModule->UnRegisterTrackEditor(DialogueSentenceTrackEditorHandle);
		SequencerModule->UnRegisterTrackEditor(CameraTrackingTrackEditorHandle);
		FGenDialogueSequenceEditor::Get().Unregister(*SequencerModule);
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

	if (UObjectInitialized())
	{
		UThumbnailManager::Get().UnregisterCustomRenderer(UAutoGenDialogueCameraTemplateAsset::StaticClass());
		UThumbnailManager::Get().UnregisterCustomRenderer(UDialogueStandPositionTemplateAsset::StaticClass());
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAutoGenSequencer_EditorModule, AutoGenSequencer_Editor)