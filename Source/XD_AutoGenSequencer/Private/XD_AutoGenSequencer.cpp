// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "XD_AutoGenSequencer.h"
#if WITH_EDITOR
#include <ISettingsModule.h>
#include <ISettingsSection.h>
#endif
#include "AutoGenDialogueSettings.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencerModule"

void FXD_AutoGenSequencerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
#if WITH_EDITOR
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
#endif //WITH_EDITOR
}

void FXD_AutoGenSequencerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FXD_AutoGenSequencerModule, XD_AutoGenSequencer)