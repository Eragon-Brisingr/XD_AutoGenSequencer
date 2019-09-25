// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoGenDialogueSequenceFactory.h"
#include "MovieSceneToolsProjectSettings.h"
#include "FrameRate.h"
#include "PreviewDialogueSoundSequence.h"
#include "LevelSequence.h"
#include "AutoGenDialogueSystemData.h"
#include "PreviewDialogueSentenceTrack.h"
#include "AutoGenDialogueSequenceConfig.h"
#include "XD_AutoGenSequencer_Editor.h"
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"
#include "SClassPickerDialog.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

UAutoGenDialogueSequenceFactory::UAutoGenDialogueSequenceFactory()
{
	SupportedClass = ULevelSequence::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UAutoGenDialogueSequenceFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	ULevelSequence* NewLevelSequence = NewObject<ULevelSequence>(InParent, Name, Flags | RF_Transactional);
	NewLevelSequence->Initialize();

	// Set up some sensible defaults
	const UMovieSceneToolsProjectSettings* ProjectSettings = GetDefault<UMovieSceneToolsProjectSettings>();
	FFrameRate TickResolution = NewLevelSequence->GetMovieScene()->GetTickResolution();
	NewLevelSequence->GetMovieScene()->SetPlaybackRange((ProjectSettings->DefaultStartTime*TickResolution).FloorToFrame(), (ProjectSettings->DefaultDuration*TickResolution).FloorToFrame().Value);

	AddAutoGenDialogueSystemData(NewLevelSequence, AutoGenDialogueSequenceConfigClass);
	AutoGenDialogueSequenceConfigClass = nullptr;

	return NewLevelSequence;
}

bool UAutoGenDialogueSequenceFactory::ConfigureProperties()
{
	AutoGenDialogueSequenceConfigClass = nullptr;

	UClass* ChosenClass = nullptr;
	const bool bPressedOk = ShowPickConfigClassViewer(ChosenClass);

	if (bPressedOk)
	{
		check(ChosenClass);
		AutoGenDialogueSequenceConfigClass = ChosenClass;
	}

	return bPressedOk; 
}

void UAutoGenDialogueSequenceFactory::AddAutoGenDialogueSystemData(ULevelSequence* LevelSequence, TSubclassOf<UGenDialogueSequenceConfigBase> AutoGenDialogueSequenceConfigClass)
{
	const EObjectFlags Flags = EObjectFlags::RF_Transactional;
	UAutoGenDialogueSystemData* AutoGenDialogueSystemData = LevelSequence->FindOrAddMetaData<UAutoGenDialogueSystemData>();
	AutoGenDialogueSystemData->SetFlags(Flags);
	AutoGenDialogueSystemData->AutoGenDialogueSequenceConfig = NewObject<UGenDialogueSequenceConfigBase>(LevelSequence, AutoGenDialogueSequenceConfigClass, GET_MEMBER_NAME_CHECKED(UAutoGenDialogueSystemData, AutoGenDialogueSequenceConfig), Flags);
	AutoGenDialogueSystemData->bIsNewCreated = true;
	AutoGenDialogueSystemData->bIsNotSetStandPosition = true;
}

bool UAutoGenDialogueSequenceFactory::ShowPickConfigClassViewer(UClass*& ChosenClass)
{
	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
	FClassViewerInitializationOptions Options;

	Options.Mode = EClassViewerMode::ClassPicker;
	Options.DisplayMode = EClassViewerDisplayMode::DefaultView;
	class FAutoGenDialogueCameraTemplateViewer : public IClassViewerFilter
	{
	public:
		EClassFlags DisallowedClassFlags = CLASS_Deprecated | CLASS_Abstract;

		virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<class FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return !InClass->HasAnyClassFlags(DisallowedClassFlags) && InClass->IsChildOf<UGenDialogueSequenceConfigBase>() != EFilterReturn::Failed;
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const class IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<class FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags) && InUnloadedClassData->IsChildOf(UGenDialogueSequenceConfigBase::StaticClass());
		}
	};
	Options.ClassFilter = MakeShareable<FAutoGenDialogueCameraTemplateViewer>(new FAutoGenDialogueCameraTemplateViewer());
	Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::Dynamic;

	const FText TitleText = LOCTEXT("选择生成对白配置", "选择生成对白配置");
	return SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UGenDialogueSequenceConfigBase::StaticClass());
}

FText UAutoGenDialogueSequenceFactory::GetDisplayName() const
{
	return LOCTEXT("创建对白定序器", "对白定序器");
}

uint32 UAutoGenDialogueSequenceFactory::GetMenuCategories() const
{
	return FXD_AutoGenSequencer_EditorModule::AutoGenDialogueSequence_AssetCategory;
}

#undef LOCTEXT_NAMESPACE
