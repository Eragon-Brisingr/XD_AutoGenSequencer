// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/AutoGenDialogueSettings.h"
#include "Engine/EngineTypes.h"
#include "Data/DialogueSentence.h"
#include "Tracks/SentenceTrack/DialogueSentenceSection.h"
#include "Datas/AutoGenDialogueSequenceConfig.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

UAutoGenDialogueSettings::UAutoGenDialogueSettings()
{
	DialogueSentenceType = UDialogueSentence::StaticClass();
}

TSubclassOf<UDialogueSentence> UAutoGenDialogueSettings::GetDialogueSentenceType()
{
	if (TSubclassOf<UDialogueSentence> DialogueSentence = Get().DialogueSentenceType.LoadSynchronous())
	{
		return DialogueSentence;
	}
	return UDialogueSentence::StaticClass();
}

#undef LOCTEXT_NAMESPACE
