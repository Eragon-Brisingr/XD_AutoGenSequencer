// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoGenDialogueCameraSet.h"
#include "XD_AutoGenSequencer_Editor.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

bool UAutoGenDialogueCameraSet::IsValid() const
{
	return CameraTemplates.Num() != 0;
}

UAutoGenDialogueCameraSetFactory::UAutoGenDialogueCameraSetFactory()
{
	SupportedClass = UAutoGenDialogueCameraSet::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UAutoGenDialogueCameraSetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	return NewObject<UAutoGenDialogueCameraSet>(InParent, Class, Name, Flags);
}

FText UAutoGenDialogueCameraSetFactory::GetDisplayName() const
{
	return LOCTEXT("创建对白镜头集", "对白镜头集");
}

uint32 UAutoGenDialogueCameraSetFactory::GetMenuCategories() const
{
	return FXD_AutoGenSequencer_EditorModule::AutoGenDialogueSequence_AssetCategory;
}

#undef LOCTEXT_NAMESPACE
