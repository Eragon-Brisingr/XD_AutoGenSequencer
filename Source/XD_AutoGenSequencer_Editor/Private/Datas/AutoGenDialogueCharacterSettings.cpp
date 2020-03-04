// Fill out your copyright notice in the Description page of Project Settings.

#include "Datas/AutoGenDialogueCharacterSettings.h"
#include "XD_AutoGenSequencer_Editor.h"
#include "Utils/AutoGenDialogueSettings.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

UAutoGenDialogueCharacterSettingsFactory::UAutoGenDialogueCharacterSettingsFactory()
{
	SupportedClass = UAutoGenDialogueCharacterSettings::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UAutoGenDialogueCharacterSettingsFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	return NewObject<UAutoGenDialogueCharacterSettings>(InParent, UAutoGenDialogueSettings::GetDefaultDialogueCharacterSettingsType(), Name, Flags);
}

FText UAutoGenDialogueCharacterSettingsFactory::GetDisplayName() const
{
	return LOCTEXT("创建对话角色配置", "创建对话角色配置");
}

FText FAssetTypeActions_AutoGenDialogueCharacterSettings::GetName() const
{
	return LOCTEXT("对话角色配置", "对话角色配置");
}

UClass* FAssetTypeActions_AutoGenDialogueCharacterSettings::GetSupportedClass() const
{
	return UAutoGenDialogueCharacterSettings::StaticClass();
}

FColor FAssetTypeActions_AutoGenDialogueCharacterSettings::GetTypeColor() const
{
	return FColor::Black;
}

uint32 FAssetTypeActions_AutoGenDialogueCharacterSettings::GetCategories()
{
	return FXD_AutoGenSequencer_EditorModule::AutoGenDialogueSequence_AssetCategory;
}

#undef LOCTEXT_NAMESPACE
