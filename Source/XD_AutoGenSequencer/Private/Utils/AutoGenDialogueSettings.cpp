// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoGenDialogueSettings.h"
#include "AssetTypeCategories.h"
#include "DialogueSentence.h"
#include "DialogueSentenceSection.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

UAutoGenDialogueSettings::UAutoGenDialogueSettings()
{
	DialogueSentenceType = UDialogueSentence::StaticClass();
	DialogueSentenceSectionType = UDialogueSentenceSection::StaticClass();
}

TSubclassOf<UDialogueSentence> UAutoGenDialogueSettings::GetDialogueSentenceType()
{
	if (TSubclassOf<UDialogueSentence> DialogueSentence = Get().DialogueSentenceType.Get())
	{
		return DialogueSentence;
	}
	return UDialogueSentence::StaticClass();
}

TSubclassOf<UDialogueSentenceSection> UAutoGenDialogueSettings::GetDialogueSentenceSectionType()
{
	if (TSubclassOf<UDialogueSentenceSection> DialogueSentenceSection = Get().DialogueSentenceSectionType.Get())
	{
		return DialogueSentenceSection;
	}
	return UDialogueSentenceSection::StaticClass();
}

#undef LOCTEXT_NAMESPACE
