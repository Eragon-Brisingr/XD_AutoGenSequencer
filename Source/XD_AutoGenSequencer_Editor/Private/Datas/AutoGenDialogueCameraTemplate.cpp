// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoGenDialogueCameraTemplate.h"
#include "DialogueStandPositionTemplate.h"
#include "Components/ChildActorComponent.h"
#include "CineCameraComponent.h"
#include "GameFramework/Character.h"
#include "CineCameraActor.h"
#include "XD_AutoGenSequencer_Editor.h"
#include "ClassViewerFilter.h"
#include "KismetCompilerModule.h"
#include "Engine/Blueprint.h"
#include "KismetEditorUtilities.h"
#include "ClassViewerModule.h"
#include "SClassPickerDialog.h"
#include "AssetEditorManager.h"
#include "BlueprintEditor.h"
#include "SEditorViewport.h"
#include "EditorViewportClient.h"
#include "EngineUtils.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

AAutoGenDialogueCameraTemplate::AAutoGenDialogueCameraTemplate()
	:bActiveCameraViewport(true)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StandTemplatePreview = CreateDefaultSubobject<UChildActorComponent>(GET_MEMBER_NAME_CHECKED(AAutoGenDialogueCameraTemplate, StandTemplatePreview));
	StandTemplatePreview->bIsEditorOnly = true;
	SetRootComponent(StandTemplatePreview);

	CineCamera = CreateDefaultSubobject<UChildActorComponent>(GET_MEMBER_NAME_CHECKED(AAutoGenDialogueCameraTemplate, CineCamera));
	CineCamera->bIsEditorOnly = true;
	CineCamera->SetChildActorClass(ACineCameraActor::StaticClass());
	CineCamera->SetupAttachment(StandTemplatePreview);
	if (ACineCameraActor* CineCameraActorTemplate = Cast<ACineCameraActor>(CineCamera->GetChildActorTemplate()))
	{
		CineCameraComponent = CineCameraActorTemplate->GetCineCameraComponent();
	}
}

void AAutoGenDialogueCameraTemplate::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ACineCameraActor* CineCameraActorTemplate = CastChecked<ACineCameraActor>(CineCamera->GetChildActorTemplate());
	CineCameraComponent = CineCameraActorTemplate->GetCineCameraComponent();
}

void AAutoGenDialogueCameraTemplate::PreEditChange(UProperty* PropertyThatWillChange)
{
	//Super::PreEditChange(PropertyThatWillChange);
}

void AAutoGenDialogueCameraTemplate::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	//ReregisterComponentsWhenModified()
	//Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AAutoGenDialogueCameraTemplate, StandTemplate))
	{
		StandTemplatePreview->SetChildActorClass(StandTemplate);
	}

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		RerunConstructionScripts();
	}

	if (bActiveCameraViewport)
	{
		if (UBlueprint* Blueprint = Cast<UBlueprint>(GetClass()->ClassGeneratedBy))
		{
			if (IAssetEditorInstance* AssetEditorInstance = FAssetEditorManager::Get().FindEditorForAsset(Blueprint, false))
			{
				FBlueprintEditor* BlueprintEditor = static_cast<FBlueprintEditor*>(AssetEditorInstance);
				TSharedPtr<FEditorViewportClient> EditorViewportClient = ((SEditorViewport*)BlueprintEditor->GetSCSViewport().Get())->GetViewportClient();

				if (TActorIterator<ACineCameraActor> It = TActorIterator<ACineCameraActor>(EditorViewportClient->GetWorld()))
				{
					ACineCameraActor* CineCameraActor = *It;
					EditorViewportClient->SetViewLocation(CineCameraActor->GetActorLocation());
					EditorViewportClient->SetViewRotation(CineCameraActor->GetActorRotation());
				}
			}
		}
	}
}

UAutoGenDialogueCameraTemplateFactory::UAutoGenDialogueCameraTemplateFactory()
{
	SupportedClass = AAutoGenDialogueCameraTemplate::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UAutoGenDialogueCameraTemplateFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	UClass* BlueprintClass = nullptr;
	UClass* BlueprintGeneratedClass = nullptr;

	IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
	KismetCompilerModule.GetBlueprintTypesForClass(CameraTemplateClass, BlueprintClass, BlueprintGeneratedClass);
	UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(CameraTemplateClass, InParent, Name, EBlueprintType::BPTYPE_Normal, BlueprintClass, BlueprintGeneratedClass);
	CameraTemplateClass = nullptr;
	return Blueprint;
}

bool UAutoGenDialogueCameraTemplateFactory::ConfigureProperties()
{
	CameraTemplateClass = nullptr;

	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
	FClassViewerInitializationOptions Options;

	Options.Mode = EClassViewerMode::ClassPicker;
	Options.DisplayMode = EClassViewerDisplayMode::TreeView;
	class FAutoGenDialogueCameraTemplateViewer : public IClassViewerFilter
	{
	public:
		EClassFlags DisallowedClassFlags = CLASS_Deprecated | CLASS_Abstract;

		virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<class FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return !InClass->HasAnyClassFlags(DisallowedClassFlags) && InClass->IsChildOf<AAutoGenDialogueCameraTemplate>() != EFilterReturn::Failed;
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const class IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<class FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags) && InUnloadedClassData->IsChildOf(AAutoGenDialogueCameraTemplate::StaticClass());
		}
	};
	Options.ClassFilter = MakeShareable<FAutoGenDialogueCameraTemplateViewer>(new FAutoGenDialogueCameraTemplateViewer());
	Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::Dynamic;

	const FText TitleText = LOCTEXT("选择对白镜头模板类型", "选择对白镜头模板类型");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, AAutoGenDialogueCameraTemplate::StaticClass());

	if (bPressedOk)
	{
		check(ChosenClass);
		CameraTemplateClass = ChosenClass;
	}

	return bPressedOk;
}

FText UAutoGenDialogueCameraTemplateFactory::GetDisplayName() const
{
	return LOCTEXT("创建对白镜头模板", "对白镜头模板");
}

uint32 UAutoGenDialogueCameraTemplateFactory::GetMenuCategories() const
{
	return FXD_AutoGenSequencer_EditorModule::AutoGenDialogueSequence_AssetCategory;
}

#undef LOCTEXT_NAMESPACE
