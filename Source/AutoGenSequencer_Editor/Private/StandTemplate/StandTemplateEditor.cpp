// Fill out your copyright notice in the Description page of Project Settings.


#include "StandTemplate/StandTemplateEditor.h"
#include <AdvancedPreviewScene.h>
#include <EditorModes.h>
#include <Editor.h>
#include <EngineUtils.h>
#include <SCommonEditorViewportToolbarBase.h>
#include <EditorModeManager.h>
#include <EditorViewportClient.h>
#include <Engine/Selection.h>

#include "StandTemplate/DialogueStandPositionTemplate.h"

#define LOCTEXT_NAMESPACE "StandTemplateEditor"

const FName FStandTemplateEditor::StandTemplateEditorDetailsTabId(TEXT("StandTemplateEditorDetailsTabId"));
const FName FStandTemplateEditor::StandTemplateEditorViewportTabId(TEXT("StandTemplateEditorViewportTabId"));

FStandTemplateEditor::FStandTemplateEditor()
{
	
}

FStandTemplateEditor::~FStandTemplateEditor()
{
	
}

FLinearColor FStandTemplateEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::Blue;
}

FName FStandTemplateEditor::GetToolkitFName() const
{
	return FName("StandTemplateEditor");
}

FText FStandTemplateEditor::GetBaseToolkitName() const
{
	return LOCTEXT("StandTemplateEditorAppLabel", "Stand Template Editor");
}

FString FStandTemplateEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "Stand Template").ToString();
}

TSharedRef<SDockTab> FStandTemplateEditor::HandleTabManagerSpawnTabDetails(const FSpawnTabArgs& Args)
{
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = true;
	DetailsViewArgs.bCustomNameAreaLocation = true;
	DetailsViewArgs.bLockable = false;
	//DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.NotifyHook = this;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	DetailsWidget = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsWidget->SetObject(Viewport->GetViewportClient()->PreviewTemplate.Get());

	return SNew(SDockTab).TabRole(ETabRole::PanelTab)[DetailsWidget.ToSharedRef()];
}

TSharedRef<SDockTab> FStandTemplateEditor::HandleTabManagerSpawnTabViewport(const FSpawnTabArgs& Args)
{
	TSharedRef<SDockTab> SpawnedTab =
		SNew(SDockTab)
		.Label(LOCTEXT("Viewport_TabTitle", "Viewport"))
		[
			Viewport.ToSharedRef()
		];

	return SpawnedTab;
}

void FStandTemplateEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("StandTemplateEditorWorkspaceMenu", "Stand Template Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	Super::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FStandTemplateEditor::StandTemplateEditorDetailsTabId, FOnSpawnTab::CreateSP(this, &FStandTemplateEditor::HandleTabManagerSpawnTabDetails))
		.SetDisplayName(LOCTEXT("StandTemplateEditorDetailsTab", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef);

	InTabManager->RegisterTabSpawner(FStandTemplateEditor::StandTemplateEditorViewportTabId, FOnSpawnTab::CreateSP(this, &FStandTemplateEditor::HandleTabManagerSpawnTabViewport))
		.SetDisplayName(LOCTEXT("StandTemplateEditorViewportTab", "Stand Template Editor Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef);
}

void FStandTemplateEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	Super::UnregisterTabSpawners(InTabManager);
	InTabManager->UnregisterTabSpawner(StandTemplateEditorDetailsTabId);
}

void FStandTemplateEditor::SaveAsset_Execute()
{
	Super::SaveAsset_Execute();

	if (Viewport.IsValid())
	{
		Viewport->GetViewportClient()->InitStandTemplateViewportClient(StandPositionTemplateAsset.Get());
		DetailsWidget->SetObject(Viewport->GetViewportClient()->PreviewTemplate.Get());
	}
}

void FStandTemplateEditor::InitStandTemplateEditor(
	const EToolkitMode::Type InMode,
	const TSharedPtr<class IToolkitHost>& InToolkitHost,
	UDialogueStandPositionTemplateAsset* InAsset
)
{
	StandPositionTemplateAsset = InAsset;

	Viewport = SNew(SStandTemplateViewport);
	Viewport->GetViewportClient()->InitStandTemplateViewportClient(InAsset);

	TSharedRef<FTabManager::FLayout>Layout = FTabManager::NewLayout("Layout_StandTemplateEditor_V1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->AddTab(FStandTemplateEditor::GetToolbarTabId(), ETabState::OpenedTab)
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
					->AddTab(FStandTemplateEditor::StandTemplateEditorViewportTabId, ETabState::OpenedTab)
					->SetHideTabWell(true)
					->SetSizeCoefficient(0.8f)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.2f)
					->SetHideTabWell(true)
					->AddTab(FStandTemplateEditor::StandTemplateEditorDetailsTabId, ETabState::OpenedTab)
				)
			)
		);

	Super::InitAssetEditor(InMode, InToolkitHost, FName("StandTemplateEditorIdentifier"), Layout, true, true, InAsset);
}

FStandTemplateViewportClient::FStandTemplateViewportClient(const TSharedRef<SStandTemplateViewport>& InThumbnailViewport, const TSharedRef<FAdvancedPreviewScene>& InPreviewScene)
	: Super(nullptr, &InPreviewScene.Get(), StaticCastSharedRef<SEditorViewport>(InThumbnailViewport))
	, ViewportPtr(InThumbnailViewport)
{
	AdvancedPreviewScene = static_cast<FAdvancedPreviewScene*>(PreviewScene);

	SetRealtime(true);

	// Hide grid, we don't need this.
	DrawHelper.bDrawGrid = false;
	DrawHelper.bDrawPivot = false;
	DrawHelper.AxesLineThickness = 5;
	DrawHelper.PivotSize = 5;

	//Initiate view
	SetViewLocation(FVector(75, 75, 75));
	SetViewRotation(FVector(-75, -75, -75).Rotation());

	EngineShowFlags.SetScreenPercentage(true);

	// Set the Default type to Ortho and the XZ Plane
	ELevelViewportType NewViewportType = LVT_Perspective;
	SetViewportType(NewViewportType);

	// View Modes in Persp and Ortho
	SetViewModes(VMI_Lit, VMI_Lit);
	
	GetModeTools()->SetDefaultMode(FBuiltinEditorModes::EM_Default);
}

void FStandTemplateViewportClient::InitStandTemplateViewportClient(UDialogueStandPositionTemplateAsset* InAsset)
{
	StandPositionTemplateAsset = InAsset;

	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.Template = InAsset->Template;
	if (PreviewTemplate.IsValid())
	{
		PreviewTemplate->Destroy();
	}
	PreviewTemplate = GetWorld()->SpawnActor<ADialogueStandPositionTemplate>(ActorSpawnParameters);
	PreviewTemplate->CreateAllTemplatePreviewCharacter();

	PreviewTemplate->OnInstanceChanged.BindLambda([=] 
	{
		// 同步属性变化
		ADialogueStandPositionTemplate* Template = InAsset->Template;
		Template->PreviewCharacter = PreviewTemplate->PreviewCharacter;
		Template->AutoGenDialogueCameraSet = PreviewTemplate->AutoGenDialogueCameraSet;
		Template->StandPositions = PreviewTemplate->StandPositions;

		InAsset->MarkPackageDirty(); 
	});
	GetModeTools()->ActivateDefaultMode();
	SetWidgetMode(FWidget::EWidgetMode::WM_Translate);
	GetModeTools()->GetSelectedActors()->Select(PreviewTemplate.Get(), true);
}

void FStandTemplateViewportClient::ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY)
{
	// 只允许选中位置属性编辑
	if (HitProxy && HitProxy->IsA(HActor::StaticGetType()))
	{
		return;
	}
	Super::ProcessClick(View, HitProxy, Key, Event, HitX, HitY);
}

void FStandTemplateViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	Super::Draw(View, PDI);

	if (PreviewTemplate.IsValid() == false)
	{
		return;
	}

	const TArray<UChildActorComponent*>& PreviewCharacters = PreviewTemplate->PreviewCharacters;
	for (int32 I = 0; I < PreviewCharacters.Num(); ++I)
	{
		for (int32 J = I + 1; J < PreviewCharacters.Num(); ++J)
		{
			const UChildActorComponent* LHS = PreviewCharacters[I];
			const UChildActorComponent* RHS = PreviewCharacters[J];
			if (LHS && RHS)
			{
				PDI->DrawLine(LHS->GetComponentLocation(), RHS->GetComponentLocation(), FColor::Green, SDPG_Foreground, 1.f);
			}
		}
	}
}

void FStandTemplateViewportClient::DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas)
{
	Super::DrawCanvas(InViewport, View, Canvas);

	if (PreviewTemplate.IsValid() == false)
	{
		return;
	}

	const TArray<UChildActorComponent*>& PreviewCharacters = PreviewTemplate->PreviewCharacters;
	for (int32 I = 0; I < PreviewCharacters.Num(); ++I)
	{
		for (int32 J = I + 1; J < PreviewCharacters.Num(); ++J)
		{
			const UChildActorComponent* LHS = PreviewCharacters[I];
			const UChildActorComponent* RHS = PreviewCharacters[J];

			const FVector Center = (LHS->GetComponentLocation() + RHS->GetComponentLocation()) * 0.5f;

			FVector2D ScreenPosition;
			if (View.ProjectWorldToScreen(Center, Canvas.GetViewRect(), View.ViewMatrices.GetViewProjectionMatrix(), ScreenPosition))
			{
				FString Desc = FString::Printf(TEXT("%.2f m"), (LHS->GetComponentLocation() - RHS->GetComponentLocation()).Size() / 100.f);
				int32 XL;
				int32 YL;
				StringSize(GEngine->GetLargeFont(), XL, YL, *Desc);

				Canvas.DrawShadowedString(ScreenPosition.X - XL * 0.5f, ScreenPosition.Y - YL * 0.5f, *Desc, GEngine->GetLargeFont(), FColor::Red);
			}
		}
	}
}

SStandTemplateViewport::SStandTemplateViewport() 
	: PreviewScene(MakeShareable(new FAdvancedPreviewScene(FPreviewScene::ConstructionValues())))
{

}

SStandTemplateViewport::~SStandTemplateViewport()
{
	if (StandTemplateViewportClient.IsValid())
	{
		StandTemplateViewportClient->Viewport = NULL;
	}
}

void SStandTemplateViewport::AddReferencedObjects(FReferenceCollector& Collector)
{

}

TSharedRef<class SEditorViewport> SStandTemplateViewport::GetViewportWidget()
{
	return SharedThis(this);
}

TSharedPtr<FExtender> SStandTemplateViewport::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}

void SStandTemplateViewport::OnFloatingButtonClicked()
{
	// Nothing
}

void SStandTemplateViewport::Construct(const FArguments& InArgs)
{
	SEditorViewport::Construct(SEditorViewport::FArguments());
}

TSharedRef<FEditorViewportClient> SStandTemplateViewport::MakeEditorViewportClient()
{
	StandTemplateViewportClient = MakeShareable(new FStandTemplateViewportClient(SharedThis(this), PreviewScene.ToSharedRef()));
	return StandTemplateViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SStandTemplateViewport::MakeViewportToolbar()
{
	class SStandTemplateEditorViewportToolbar : public SCommonEditorViewportToolbarBase
	{
		using Super = SCommonEditorViewportToolbarBase;
	public:
		void Construct(const FArguments& InArgs, TSharedPtr<class ICommonEditorViewportToolbarInfoProvider> InInfoProvider)
		{
			Super::Construct(InArgs, InInfoProvider);
		}
	};
	return SNew(SStandTemplateEditorViewportToolbar, SharedThis(this));
}

#undef LOCTEXT_NAMESPACE
