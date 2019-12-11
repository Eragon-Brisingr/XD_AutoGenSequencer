// Fill out your copyright notice in the Description page of Project Settings.


#include "Factory/DialogueSentenceFactory.h"
#include "Data/DialogueSentence.h"
#include "Utils/AutoGenDialogueSettings.h"
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

FText FAssetTypeActions_DialogueSentence::GetName() const
{
	return LOCTEXT("对白语句", "对白语句");
}

UClass* FAssetTypeActions_DialogueSentence::GetSupportedClass() const
{
	return UDialogueSentence::StaticClass();
}

FColor FAssetTypeActions_DialogueSentence::GetTypeColor() const
{
	return FColor::Black;
}

uint32 FAssetTypeActions_DialogueSentence::GetCategories()
{
	return FXD_AutoGenSequencer_EditorModule::AutoGenDialogueSequence_AssetCategory;
}

FText FAssetTypeActions_DialogueSentence::GetDisplayNameFromAssetData(const FAssetData& AssetData) const
{
	FString SubTitle;
	if (AssetData.GetTagValue<FString>(UDialogueSentence::AssetRegistryTag_SubTitle, SubTitle))
	{
		return FText::FromString(SubTitle);
	}
	return Super::GetDisplayNameFromAssetData(AssetData);
}

#undef LOCTEXT_NAMESPACE
