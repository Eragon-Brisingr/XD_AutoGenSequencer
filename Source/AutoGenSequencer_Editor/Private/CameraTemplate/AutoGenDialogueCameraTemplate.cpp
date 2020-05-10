// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraTemplate/AutoGenDialogueCameraTemplate.h"
#include <Components/ChildActorComponent.h>
#include <CineCameraComponent.h>
#include <GameFramework/Character.h>
#include <CineCameraActor.h>
#include <ClassViewerFilter.h>
#include <ClassViewerModule.h>
#include <Kismet2/SClassPickerDialog.h>
#include <Toolkits/AssetEditorManager.h>
#include <EngineUtils.h>
#include <KismetCompilerModule.h>
#include <Kismet2/KismetEditorUtilities.h>
#include <AdvancedPreviewScene.h>

#include "StandTemplate/DialogueStandPositionTemplate.h"
#include "AutoGenSequencer_Editor.h"
#include "CameraTemplate/CameraTemplateEditor.h"

#define LOCTEXT_NAMESPACE "FAutoGenSequencer_EditorModule"

AAutoGenDialogueCameraTemplate::AAutoGenDialogueCameraTemplate()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TemplateRoot = CreateDefaultSubobject<USceneComponent>(GET_MEMBER_NAME_CHECKED(AAutoGenDialogueCameraTemplate, TemplateRoot));
	SetRootComponent(TemplateRoot);

	CineCameraActorTemplate = CreateDefaultSubobject<ACineCameraActor>(GET_MEMBER_NAME_CHECKED(AAutoGenDialogueCameraTemplate, CineCameraActorTemplate));
	CineCameraComponent = CineCameraActorTemplate->GetCineCameraComponent();
}

void AAutoGenDialogueCameraTemplate::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Template = CineCameraActorTemplate;
	SpawnParameters.ObjectFlags = RF_Transient;
	CineCameraActor = GetWorld()->SpawnActor<ACineCameraActor>(SpawnParameters);
	CineCameraComponent = CineCameraActor->GetCineCameraComponent();

	UpdateCameraTransform();
}

void AAutoGenDialogueCameraTemplate::Destroyed()
{
	Super::Destroyed();

	CineCameraActor->Destroy();
}

void AAutoGenDialogueCameraTemplate::PreEditChange(FProperty* PropertyThatWillChange)
{
	// Hack！添加这个防止属性修改时组件反复创建
	SetFlags(RF_ArchetypeObject);
	Super::PreEditChange(PropertyThatWillChange);
	ClearFlags(RF_ArchetypeObject);
}

void AAutoGenDialogueCameraTemplate::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	SetFlags(RF_ArchetypeObject);
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ClearFlags(RF_ArchetypeObject);

	if (CineCameraActor)
	{
		UpdateCameraTransform();
	}
}

UAutoGenDialogueCameraTemplateFactory::UAutoGenDialogueCameraTemplateFactory()
{
	SupportedClass = UAutoGenDialogueCameraTemplateAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UAutoGenDialogueCameraTemplateFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	UAutoGenDialogueCameraTemplateAsset* Asset = NewObject<UAutoGenDialogueCameraTemplateAsset>(InParent, Name, Flags);
	Asset->Template = NewObject<AAutoGenDialogueCameraTemplate>(Asset, CameraTemplateClass, Name, RF_Public | RF_Transactional);
	return Asset;
}

bool UAutoGenDialogueCameraTemplateFactory::ConfigureProperties()
{
	CameraTemplateClass = nullptr;

	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
	FClassViewerInitializationOptions Options;

	Options.Mode = EClassViewerMode::ClassPicker;
	Options.DisplayMode = EClassViewerDisplayMode::ListView;
	class FAutoGenDialogueCameraTemplateViewer : public IClassViewerFilter
	{
	public:
		EClassFlags DisallowedClassFlags = CLASS_Deprecated | CLASS_Abstract;

		virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<class FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return !InClass->HasAnyClassFlags(DisallowedClassFlags) && InClass->IsChildOf<AAutoGenDialogueCameraTemplate>() != EFilterReturn::Failed && InClass->HasAnyClassFlags(CLASS_Native);
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const class IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<class FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags) && InUnloadedClassData->IsChildOf(AAutoGenDialogueCameraTemplate::StaticClass()) && InUnloadedClassData->HasAnyClassFlags(CLASS_Native);
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

FText FAssetTypeActions_AutoGenDialogueCameraTemplate::GetName() const
{
	return LOCTEXT("对白镜头模板", "对白镜头模板");
}

UClass* FAssetTypeActions_AutoGenDialogueCameraTemplate::GetSupportedClass() const
{
	return UAutoGenDialogueCameraTemplateAsset::StaticClass();
}

FColor FAssetTypeActions_AutoGenDialogueCameraTemplate::GetTypeColor() const
{
	return FColor::Black;
}

uint32 FAssetTypeActions_AutoGenDialogueCameraTemplate::GetCategories()
{
	return FAutoGenSequencer_EditorModule::AutoGenDialogueSequence_AssetCategory;
}

void FAssetTypeActions_AutoGenDialogueCameraTemplate::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	for (UObject* Object : InObjects)
	{
		if (UAutoGenDialogueCameraTemplateAsset* Asset = Cast<UAutoGenDialogueCameraTemplateAsset>(Object))
		{
			TSharedRef<FCameraTemplateEditor> EditorToolkit = MakeShareable(new FCameraTemplateEditor());
			EditorToolkit->InitCameraTemplateEditor(Mode, EditWithinLevelEditor, Asset);
		}
	}
}

void UAutoGenDialogueCameraTemplateThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	if (UAutoGenDialogueCameraTemplateAsset* Asset = Cast<UAutoGenDialogueCameraTemplateAsset>(Object))
	{
		if (ThumbnailScene == nullptr)
		{
			ThumbnailScene = new FAdvancedPreviewScene(FPreviewScene::ConstructionValues());
		}

		if (AAutoGenDialogueCameraTemplate* Template = Asset->Template)
		{
			FActorSpawnParameters ActorSpawnParameters;
			ActorSpawnParameters.Template = Template;
			ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ActorSpawnParameters.bNoFail = true;
			ActorSpawnParameters.ObjectFlags = RF_Transient;
			AAutoGenDialogueCameraTemplate* PreviewCameraTemplate = ThumbnailScene->GetWorld()->SpawnActor<AAutoGenDialogueCameraTemplate>(Template->GetClass(), ActorSpawnParameters);
			PreviewCameraTemplate->UpdateCameraTransform();

			{
				UCameraComponent* CineCameraComponent = PreviewCameraTemplate->CineCameraComponent;

				FMinimalViewInfo ViewInfo;
				CineCameraComponent->GetCameraView(FApp::GetDeltaTime(), ViewInfo);

				UWorld* World = CineCameraComponent->GetWorld();

				FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(RenderTarget, ThumbnailScene->GetScene(), FEngineShowFlags(ESFIM_Game))
					.SetWorldTimes(FApp::GetCurrentTime() - GStartTime, FApp::GetDeltaTime(), FApp::GetCurrentTime() - GStartTime)
					.SetResolveScene(true));

				FSceneViewInitOptions ViewInitOptions;

				ViewInitOptions.BackgroundColor = FLinearColor::Black;
				const float YOffset = (Height - Height / ViewInfo.AspectRatio) * 0.5f;
				ViewInitOptions.SetViewRectangle(FIntRect(0, YOffset, Width, Height / ViewInfo.AspectRatio + YOffset));
				ViewInitOptions.ViewFamily = &ViewFamily;

				ViewInitOptions.ViewOrigin = ViewInfo.Location;
				ViewInitOptions.ViewRotationMatrix = FInverseRotationMatrix(ViewInfo.Rotation) * FMatrix(
					FPlane(0, 0, 1, 0),
					FPlane(1, 0, 0, 0),
					FPlane(0, 1, 0, 0),
					FPlane(0, 0, 0, 1));

				ViewInitOptions.ProjectionMatrix = ViewInfo.CalculateProjectionMatrix();

				FSceneView* NewView = new FSceneView(ViewInitOptions);
				ViewFamily.Views.Add(NewView);

				RenderViewFamily(Canvas, &ViewFamily);
			}

			PreviewCameraTemplate->Destroy();
		}
	}
}

void UAutoGenDialogueCameraTemplateThumbnailRenderer::BeginDestroy()
{
	if (ThumbnailScene != nullptr)
	{
		delete ThumbnailScene;
		ThumbnailScene = nullptr;
	}

	Super::BeginDestroy();
}

#undef LOCTEXT_NAMESPACE
