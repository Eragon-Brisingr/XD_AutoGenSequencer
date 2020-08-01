// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraTemplate/CameraTemplateEditor.h"
#include <AdvancedPreviewScene.h>
#include <EditorModes.h>
#include <Editor.h>
#include <EngineUtils.h>
#include <CineCameraComponent.h>
#include <Widgets/SBoxPanel.h>
#include <Widgets/Input/SButton.h>
#include <SCommonEditorViewportToolbarBase.h>

#include "CameraTemplate/AutoGenDialogueCameraTemplate.h"

#define LOCTEXT_NAMESPACE "CameraTemplateEditor"

const FName FCameraTemplateEditor::CameraTemplateEditorDetailsTabId(TEXT("CameraTemplateEditorDetailsTabId"));
const FName FCameraTemplateEditor::CameraTemplateEditorViewportTabId(TEXT("CameraTemplateEditorViewportTabId"));

FCameraTemplateEditor::FCameraTemplateEditor()
{

}

FCameraTemplateEditor::~FCameraTemplateEditor()
{

}

FLinearColor FCameraTemplateEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::Blue;
}

FName FCameraTemplateEditor::GetToolkitFName() const
{
	return FName("CameraTemplateEditor");
}

FText FCameraTemplateEditor::GetBaseToolkitName() const
{
	return LOCTEXT("CameraTemplateEditorAppLabel", "Camera Template Editor");
}

FString FCameraTemplateEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "Camera Template").ToString();
}

void FCameraTemplateEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("CameraTemplateEditorWorkspaceMenu", "Stand Template Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	Super::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FCameraTemplateEditor::CameraTemplateEditorDetailsTabId, FOnSpawnTab::CreateSP(this, &FCameraTemplateEditor::HandleTabManagerSpawnTabDetails))
		.SetDisplayName(LOCTEXT("CameraTemplateEditorDetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef);

	InTabManager->RegisterTabSpawner(FCameraTemplateEditor::CameraTemplateEditorViewportTabId, FOnSpawnTab::CreateSP(this, &FCameraTemplateEditor::HandleTabManagerSpawnTabViewport))
		.SetDisplayName(LOCTEXT("CameraTemplateEditorViewportTab", "Stand Template Editor Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef);
}

void FCameraTemplateEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	Super::UnregisterTabSpawners(InTabManager);
	InTabManager->UnregisterTabSpawner(CameraTemplateEditorDetailsTabId);
}

void FCameraTemplateEditor::SaveAsset_Execute()
{
	Super::SaveAsset_Execute();
	if (Viewport.IsValid() == false)
	{
		return;
	}

	if (UAutoGenDialogueCameraTemplateAsset* Asset = CameraTemplateAsset.Get())
	{
		if (AAutoGenDialogueCameraTemplate* PreviewCameraTemplate = Viewport->CameraTemplateViewportClient->PreviewCameraTemplate.Get())
		{
			AAutoGenDialogueCameraTemplate* TemplateToSave = DuplicateObject(PreviewCameraTemplate, Asset);
			TemplateToSave->CineCameraActorTemplate = DuplicateObject(PreviewCameraTemplate->CineCameraActor, Asset);
			TemplateToSave->ClearFlags(RF_AllFlags);
			TemplateToSave->SetFlags(RF_Public | RF_Transactional);
			Asset->Template = TemplateToSave;
		}
	}
}

void FCameraTemplateEditor::InitCameraTemplateEditor(const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost>& InToolkitHost, UAutoGenDialogueCameraTemplateAsset* InAsset)
{
	CameraTemplateAsset = InAsset;

	Viewport = SNew(SCameraTemplateViewport);
	Viewport->GetViewportClient()->InitCameraTemplateViewportClient(InAsset);

	TSharedRef<FTabManager::FLayout>Layout = FTabManager::NewLayout("Layout_CameraTemplateEditor_V1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->AddTab(FCameraTemplateEditor::GetToolbarTabId(), ETabState::OpenedTab)
				->SetHideTabWell(true)
				->SetSizeCoefficient(0.1f)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.9f)
				->Split
				(
					FTabManager::NewStack()
					->AddTab(FCameraTemplateEditor::CameraTemplateEditorViewportTabId, ETabState::OpenedTab)
					->SetHideTabWell(true)
					->SetSizeCoefficient(0.8f)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.2f)
					->SetHideTabWell(true)
					->AddTab(FCameraTemplateEditor::CameraTemplateEditorDetailsTabId, ETabState::OpenedTab)
				)
			)
		);

	Super::InitAssetEditor(InMode, InToolkitHost, FName("CameraTemplateEditorIdentifier"), Layout, true, true, InAsset);
}

TSharedRef<SDockTab> FCameraTemplateEditor::HandleTabManagerSpawnTabDetails(const FSpawnTabArgs& Args)
{
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = true;
	DetailsViewArgs.bCustomNameAreaLocation = true;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::ENameAreaSettings::HideNameArea;
	DetailsViewArgs.NotifyHook = this;
	DetailsViewArgs.bShowActorLabel = true;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	DetailsWidget = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsWidget->SetObject(Viewport->GetViewportClient()->PreviewCameraTemplate.Get());
	DetailsWidget->OnFinishedChangingProperties().AddLambda([=](const FPropertyChangedEvent& Event)
	{
		if (CameraTemplateAsset.IsValid())
		{
			CameraTemplateAsset->MarkPackageDirty();
		}
	});

	return SNew(SDockTab).TabRole(ETabRole::PanelTab)[DetailsWidget.ToSharedRef()];
}

TSharedRef<SDockTab> FCameraTemplateEditor::HandleTabManagerSpawnTabViewport(const FSpawnTabArgs& Args)
{
	TSharedRef<SDockTab> SpawnedTab =
		SNew(SDockTab)
		.Label(LOCTEXT("Viewport_TabTitle", "Viewport"))
		[
			Viewport.ToSharedRef()
		];

	return SpawnedTab;
}

FCameraTemplateViewportClient::FCameraTemplateViewportClient(const TSharedRef<SCameraTemplateViewport>& InViewport, const TSharedRef<FAdvancedPreviewScene>& InPreviewScene)
	: Super(nullptr, &InPreviewScene.Get(), StaticCastSharedRef<SEditorViewport>(InViewport))
	, ViewportPtr(InViewport)
{
	AdvancedPreviewScene = static_cast<FAdvancedPreviewScene*>(PreviewScene);

	// Hide grid, we don't need this.
	DrawHelper.bDrawGrid = false;
	DrawHelper.bDrawPivot = false;
	DrawHelper.AxesLineThickness = 5;
	DrawHelper.PivotSize = 5;

	//Initiate view
	SetViewLocation(FVector(75, 75, 75));
	SetViewRotation(FVector(-75, -75, -75).Rotation());

	SetRealtime(true);
	EngineShowFlags.SetScreenPercentage(true);
	SetGameView(true);

	// Set the Default type to Ortho and the XZ Plane
	SetViewportType(LVT_Perspective);

	// View Modes in Persp and Ortho
	SetViewModes(VMI_Lit, VMI_Lit);
}

void FCameraTemplateViewportClient::InitCameraTemplateViewportClient(UAutoGenDialogueCameraTemplateAsset* InAsset)
{
	CameraTemplateAsset = InAsset;

	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.Template = InAsset->Template;
	PreviewCameraTemplate = GetWorld()->SpawnActor<AAutoGenDialogueCameraTemplate>(InAsset->Template->GetClass(), ActorSpawnParameters);
}

void FCameraTemplateViewportClient::ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY)
{
	// 只允许选中位置属性编辑
// 	if (HitProxy && HitProxy->IsA(HActor::StaticGetType()))
// 	{
// 		return;
// 	}
// 	Super::ProcessClick(View, HitProxy, Key, Event, HitX, HitY);
}

void FCameraTemplateViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	Super::Draw(View, PDI);
}

void FCameraTemplateViewportClient::DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas)
{
	Super::DrawCanvas(InViewport, View, Canvas);
}

void FCameraTemplateViewportClient::Tick(float DeltaSeconds)
{
	if (PreviewCameraTemplate.IsValid())
	{
		bUseControllingActorViewInfo = true;

		UCineCameraComponent* CameraComponent = PreviewCameraTemplate->CineCameraComponent;
		CameraComponent->GetCameraView(0.f, ControllingActorViewInfo);
		CameraComponent->GetExtraPostProcessBlends(ControllingActorExtraPostProcessBlends, ControllingActorExtraPostProcessBlendWeights);
		ViewFOV = ControllingActorViewInfo.FOV;
		AspectRatio = ControllingActorViewInfo.AspectRatio;
		SetViewLocation(ControllingActorViewInfo.Location);
		SetViewRotation(ControllingActorViewInfo.Rotation);
	}
//	Super::Tick(DeltaSeconds);
}

bool FCameraTemplateViewportClient::GetActiveSafeFrame(float& OutAspectRatio) const
{
	if (PreviewCameraTemplate.IsValid())
	{
		const UCameraComponent* CameraComponent = PreviewCameraTemplate->CineCameraComponent;
		OutAspectRatio = CameraComponent->AspectRatio;
	}
	return true;
}

ELevelViewportType FCameraTemplateViewportClient::GetViewportType() const
{
	if (PreviewCameraTemplate.IsValid())
	{
		const UCameraComponent* CameraComponent = PreviewCameraTemplate->CineCameraComponent;
		return (CameraComponent->ProjectionMode == ECameraProjectionMode::Perspective) ? LVT_Perspective : LVT_OrthoFreelook;
	}
	return LVT_Perspective;
}

void FCameraTemplateViewportClient::OverridePostProcessSettings(FSceneView& View)
{
	if (PreviewCameraTemplate.IsValid())
	{
		const UCameraComponent* CameraComponent = PreviewCameraTemplate->CineCameraComponent;
		if (CameraComponent)
		{
			View.OverridePostProcessSettings(CameraComponent->PostProcessSettings, CameraComponent->PostProcessBlendWeight);
		}
	}
}

SCameraTemplateViewport::SCameraTemplateViewport()
	: PreviewScene(MakeShareable(new FAdvancedPreviewScene(FPreviewScene::ConstructionValues())))
{

}

SCameraTemplateViewport::~SCameraTemplateViewport()
{
	if (CameraTemplateViewportClient.IsValid())
	{
		CameraTemplateViewportClient->Viewport = NULL;
	}
}

void SCameraTemplateViewport::Construct(const FArguments& InArgs)
{
	SEditorViewport::Construct(SEditorViewport::FArguments());
}

TSharedRef<class SEditorViewport> SCameraTemplateViewport::GetViewportWidget()
{
	return SharedThis(this);
}

TSharedPtr<FExtender> SCameraTemplateViewport::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}

void SCameraTemplateViewport::OnFloatingButtonClicked()
{

}

TSharedRef<FEditorViewportClient> SCameraTemplateViewport::MakeEditorViewportClient()
{
	CameraTemplateViewportClient = MakeShareable(new FCameraTemplateViewportClient(SharedThis(this), PreviewScene.ToSharedRef()));
	return CameraTemplateViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SCameraTemplateViewport::MakeViewportToolbar()
{
	class SCameraTemplateEditorViewportToolbar : public SCommonEditorViewportToolbarBase
	{
		using Super = SCommonEditorViewportToolbarBase;
	public:
		void Construct(const FArguments& InArgs, TSharedPtr<class ICommonEditorViewportToolbarInfoProvider> InInfoProvider)
		{
			Super::Construct(InArgs, InInfoProvider);
		}
	};
	return SNew(SCameraTemplateEditorViewportToolbar, SharedThis(this));
}

#undef LOCTEXT_NAMESPACE
