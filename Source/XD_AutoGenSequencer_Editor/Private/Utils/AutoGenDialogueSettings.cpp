// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/AutoGenDialogueSettings.h"
#include <Engine/EngineTypes.h>

#include "Data/DialogueSentence.h"
#include "Tracks/SentenceTrack/DialogueSentenceSection.h"
#include "Datas/AutoGenDialogueCharacterSettings.h"
#include "Datas/AutoGenDialogueAnimSet.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

UAutoGenDialogueSettings::UAutoGenDialogueSettings()
{
	DialogueSentenceType = UDialogueSentence::StaticClass();
	DialogueCharacterSettingsType = UAutoGenDialogueCharacterSettings::StaticClass();
	AutoGenDialogueAnimSetType = UAutoGenDialogueAnimSet::StaticClass();
}

TSubclassOf<UDialogueSentence> UAutoGenDialogueSettings::GetDialogueSentenceType()
{
	static TSubclassOf<UDialogueSentence> DialogueSentence = Get().DialogueSentenceType.LoadSynchronous();
	return DialogueSentence;
}

TSubclassOf<UAutoGenDialogueCharacterSettings> UAutoGenDialogueSettings::GetDialogueCharacterSettingsType()
{
	static TSubclassOf<UAutoGenDialogueCharacterSettings> DialogueCharacterSettingsType = Get().DialogueCharacterSettingsType.LoadSynchronous();
	return DialogueCharacterSettingsType;
}


#undef LOCTEXT_NAMESPACE
