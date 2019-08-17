// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "XD_AutoGenSequencer_Editor.h"
#include "ISequencerModule.h"
#include "AutoGenSequencerCBExtensions.h"
#include "DialogueSentenceTrackEditor.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

void FXD_AutoGenSequencer_EditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FAutoGenSequencerContentBrowserExtensions::RegisterExtender();

	ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
	DialogueSentenceTrackEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateLambda([](TSharedRef<ISequencer> InSequencer)
		{
			return MakeShareable(new FDialogueSentenceTrackEditor(InSequencer));
		}));
}

void FXD_AutoGenSequencer_EditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FAutoGenSequencerContentBrowserExtensions::UnregisterExtender();

	if (FModuleManager::Get().IsModuleLoaded("Sequencer"))
	{
		ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
		SequencerModule.UnRegisterTrackEditor(DialogueSentenceTrackEditorHandle);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FXD_AutoGenSequencer_EditorModule, XD_AutoGenSequencer_Editor)