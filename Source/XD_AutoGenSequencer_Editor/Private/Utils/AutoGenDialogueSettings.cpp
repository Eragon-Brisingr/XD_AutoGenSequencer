// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/AutoGenDialogueSettings.h"
#include "Engine/EngineTypes.h"
#include "Data/DialogueSentence.h"
#include "Tracks/SentenceTrack/DialogueSentenceSection.h"
#include "Datas/AutoGenDialogueCharacterSettings.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

UAutoGenDialogueSettings::UAutoGenDialogueSettings()
{
	DialogueSentenceType = UDialogueSentence::StaticClass();
	DefaultDialogueCharacterSettingsType = UAutoGenDialogueCharacterSettings::StaticClass();
}

TSubclassOf<UDialogueSentence> UAutoGenDialogueSettings::GetDialogueSentenceType()
{
	static TSubclassOf<UDialogueSentence> DialogueSentence = Get().DialogueSentenceType.LoadSynchronous();
	return DialogueSentence;
}

TSubclassOf<UAutoGenDialogueCharacterSettings> UAutoGenDialogueSettings::GetDefaultDialogueCharacterSettingsType()
{
	static TSubclassOf<UAutoGenDialogueCharacterSettings> DialogueCharacterSettingsType = Get().DefaultDialogueCharacterSettingsType.LoadSynchronous();
	return DialogueCharacterSettingsType;
}


#undef LOCTEXT_NAMESPACE
