// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoGenDialogueAnimSet.h"

#include "XD_AutoGenSequencer_Editor.h"
#include "ModuleManager.h"
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"
#include "SClassPickerDialog.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

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
	SupportedClass = UAutoGenDialogueAnimSet::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UAutoGenDialogueAnimSetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	UAutoGenDialogueAnimSetBase* AutoGenDialogueAnimSet = NewObject<UAutoGenDialogueAnimSetBase>(InParent, AutoGenDialogueAnimSetClass, Name, Flags);
	AutoGenDialogueAnimSetClass = nullptr;
	return AutoGenDialogueAnimSet;
}

bool UAutoGenDialogueAnimSetFactory::ConfigureProperties()
{
	AutoGenDialogueAnimSetClass = nullptr;

	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
	FClassViewerInitializationOptions Options;

	Options.Mode = EClassViewerMode::ClassPicker;
	Options.DisplayMode = EClassViewerDisplayMode::DefaultView;
	class FAutoGenDialogueAnimSetViewer : public IClassViewerFilter
	{
	public:
		EClassFlags DisallowedClassFlags = CLASS_Deprecated | CLASS_Abstract;

		virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<class FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return !InClass->HasAnyClassFlags(DisallowedClassFlags) && InClass->IsChildOf<UAutoGenDialogueAnimSetBase>() != EFilterReturn::Failed;
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const class IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<class FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags) && InUnloadedClassData->IsChildOf(UAutoGenDialogueAnimSetBase::StaticClass());
		}
	};
	Options.ClassFilter = MakeShareable<FAutoGenDialogueAnimSetViewer>(new FAutoGenDialogueAnimSetViewer());
	Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::Dynamic;

	const FText TitleText = LOCTEXT("选择对白动画集类型", "选择对白动画集类型");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UAutoGenDialogueAnimSetBase::StaticClass());

	if (bPressedOk)
	{
		check(ChosenClass);
		AutoGenDialogueAnimSetClass = ChosenClass;
	}

	return bPressedOk;
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
