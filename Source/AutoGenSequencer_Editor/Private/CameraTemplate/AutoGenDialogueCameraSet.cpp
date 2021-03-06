﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraTemplate/AutoGenDialogueCameraSet.h"
#include "AutoGenSequencer_Editor.h"

#define LOCTEXT_NAMESPACE "FAutoGenSequencer_EditorModule"

bool UAutoGenDialogueCameraSet::IsValid(TArray<FText>& ErrorMessages) const
{
	bool bIsSucceed = true;
	if (CameraTemplates.Num() == 0)
	{
		ErrorMessages.Add(LOCTEXT("镜头模板数量为空", "镜头模板数量为空，请添加镜头"));
		bIsSucceed &= false;
	}
	else
	{
		for (const FAutoGenDialogueCameraConfig& CameraConfig : CameraTemplates)
		{
			if (CameraConfig.CameraTemplate == nullptr)
			{
				ErrorMessages.Add(LOCTEXT("镜头模板存在空的配置", "镜头模板存在空的配置，请设置"));
				bIsSucceed &= false;
				break;
			}
		}
	}
	return bIsSucceed;
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
	return FAutoGenSequencer_EditorModule::AutoGenDialogueSequence_AssetCategory;
}

FText FAssetTypeActions_AutoGenDialogueCameraSet::GetName() const
{
	return LOCTEXT("对白镜头集", "对白镜头集");
}

UClass* FAssetTypeActions_AutoGenDialogueCameraSet::GetSupportedClass() const
{
	return UAutoGenDialogueCameraSet::StaticClass();
}

FColor FAssetTypeActions_AutoGenDialogueCameraSet::GetTypeColor() const
{
	return FColor::Black;
}

uint32 FAssetTypeActions_AutoGenDialogueCameraSet::GetCategories()
{
	return FAutoGenSequencer_EditorModule::AutoGenDialogueSequence_AssetCategory;
}

#undef LOCTEXT_NAMESPACE
