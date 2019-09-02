// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueSentenceFactory.h"
#include "AssetTypeCategories.h"
#include "DialogueSentence.h"
#include "AutoGenDialogueSettings.h"

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
	return LOCTEXT("创建对话语句", "对话语句");
}

uint32 UDialogueSentenceFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Animation;
}

#undef LOCTEXT_NAMESPACE
