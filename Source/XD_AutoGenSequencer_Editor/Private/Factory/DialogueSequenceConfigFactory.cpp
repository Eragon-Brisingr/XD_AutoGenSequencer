// Fill out your copyright notice in the Description page of Project Settings.


#include "Factory/DialogueSequenceConfigFactory.h"
#include "Datas/AutoGenDialogueSequenceConfig.h"
#include "XD_AutoGenSequencer_Editor.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

UDialogueSequenceConfigFactory::UDialogueSequenceConfigFactory()
{
	SupportedClass = UAutoGenDialogueSequenceConfig::StaticClass();
	bCreateNew = false;
	bEditAfterNew = true;
}

UObject* UDialogueSequenceConfigFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UAutoGenDialogueSequenceConfig>(InParent, InClass, InName, Flags);
}

FText UDialogueSequenceConfigFactory::GetDisplayName() const
{
	return LOCTEXT("创建自动生成对白定序器配置", "生成对白定序器配置");
}

uint32 UDialogueSequenceConfigFactory::GetMenuCategories() const
{
	return FXD_AutoGenSequencer_EditorModule::AutoGenDialogueSequence_AssetCategory;
}

#undef LOCTEXT_NAMESPACE
