// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueSequenceConfigFactory.h"
#include "AutoGenDialogueSequenceConfig.h"
#include "AssetTypeCategories.h"

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
	return LOCTEXT("创建自动生成对话定序器配置", "生成对话定序器配置");
}

uint32 UDialogueSequenceConfigFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Animation;
}

#undef LOCTEXT_NAMESPACE
