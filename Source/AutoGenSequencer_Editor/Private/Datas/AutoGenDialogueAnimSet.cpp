// Fill out your copyright notice in the Description page of Project Settings.


#include "Datas/AutoGenDialogueAnimSet.h"
#include <Modules/ModuleManager.h>
#include <ClassViewerModule.h>
#include <ClassViewerFilter.h>
#include <Kismet2/SClassPickerDialog.h>

#include "AutoGenSequencer_Editor.h"
#include "Utils/AutoGenDialogueSettings.h"

#define LOCTEXT_NAMESPACE "FAutoGenSequencer_EditorModule"

bool UAutoGenDialogueAnimSetBase::IsAnimSetValid(TArray<FText>& ErrorMessages) const
{
	return false;
}

bool UAutoGenDialogueAnimSet::IsAnimSetValid(TArray<FText>& ErrorMessages) const
{
	bool bIsSucceed = true;
	if (IdleAnims.Num() == 0)
	{
		ErrorMessages.Add(LOCTEXT("站立动画集为空", "站立动画集为空"));
		bIsSucceed &= false;
	}
	else
	{
		if (IdleAnims.Contains(nullptr))
		{
			ErrorMessages.Add(LOCTEXT("站立动画集中存在空动画", "站立动画集中存在空动画，请修改"));
			bIsSucceed &= false;
		}
	}

	if (TalkAnims.Num() == 0)
	{
		ErrorMessages.Add(LOCTEXT("对白动画集为空", "对白动画集为空"));
		bIsSucceed &= false;
	}
	else
	{
		if (TalkAnims.Contains(nullptr))
		{
			ErrorMessages.Add(LOCTEXT("对白动画集中存在空动画", "对白动画集中存在空动画，请修改"));
			bIsSucceed &= false;
		}
	}

	return bIsSucceed;
}

UAutoGenDialogueAnimSetFactory::UAutoGenDialogueAnimSetFactory()
{
	SupportedClass = UAutoGenDialogueAnimSetBase::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UAutoGenDialogueAnimSetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	UAutoGenDialogueAnimSetBase* AutoGenDialogueAnimSet = NewObject<UAutoGenDialogueAnimSetBase>(InParent, UAutoGenDialogueSettings::Get().AutoGenDialogueAnimSetType.LoadSynchronous(), Name, Flags);
	return AutoGenDialogueAnimSet;
}

FText UAutoGenDialogueAnimSetFactory::GetDisplayName() const
{
	return LOCTEXT("创建对白动作集", "对白动作集");
}

uint32 UAutoGenDialogueAnimSetFactory::GetMenuCategories() const
{
	return FAutoGenSequencer_EditorModule::AutoGenDialogueSequence_AssetCategory;
}

FText FAssetTypeActions_AutoGenDialogueAnimSet::GetName() const
{
	return LOCTEXT("对白动作集", "对白动作集");
}

UClass* FAssetTypeActions_AutoGenDialogueAnimSet::GetSupportedClass() const
{
	return UAutoGenDialogueAnimSetBase::StaticClass();
}

FColor FAssetTypeActions_AutoGenDialogueAnimSet::GetTypeColor() const
{
	return FColor::Black;
}

uint32 FAssetTypeActions_AutoGenDialogueAnimSet::GetCategories()
{
	return FAutoGenSequencer_EditorModule::AutoGenDialogueSequence_AssetCategory;
}

#undef LOCTEXT_NAMESPACE
