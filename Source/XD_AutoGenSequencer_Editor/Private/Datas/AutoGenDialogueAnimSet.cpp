// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoGenDialogueAnimSet.h"
#include "XD_AutoGenSequencer_Editor.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

UAutoGenDialogueAnimSetFactory::UAutoGenDialogueAnimSetFactory()
{
	SupportedClass = UAutoGenDialogueAnimSet::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UAutoGenDialogueAnimSetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	return NewObject<UAutoGenDialogueAnimSet>(InParent, Class, Name, Flags);
}

FText UAutoGenDialogueAnimSetFactory::GetDisplayName() const
{
	return LOCTEXT("创建对白动作集", "对白动作集");
}

uint32 UAutoGenDialogueAnimSetFactory::GetMenuCategories() const
{
	return FXD_AutoGenSequencer_EditorModule::AutoGenDialogueSequence_AssetCategory;
}

#undef LOCTEXT_NAMESPACE
