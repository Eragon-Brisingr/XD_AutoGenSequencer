// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueSentenceFactory.h"
#include "DialogueSentence.h"
#include "AutoGenDialogueSettings.h"
#include "XD_AutoGenSequencer_Editor.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

UDialogueSentenceFactory::UDialogueSentenceFactory()
{
	SupportedClass = UDialogueSentence::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UDialogueSentenceFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	return NewObject<UDialogueSentence>(InParent, UAutoGenDialogueSettings::GetDialogueSentenceType(), Name, Flags);
}

FText UDialogueSentenceFactory::GetDisplayName() const
{
	return LOCTEXT("创建对白语句", "对白语句");
}

uint32 UDialogueSentenceFactory::GetMenuCategories() const
{
	return FXD_AutoGenSequencer_EditorModule::AutoGenDialogueSequence_AssetCategory;
}

#undef LOCTEXT_NAMESPACE
