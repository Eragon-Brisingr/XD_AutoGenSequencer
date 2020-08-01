// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "SEditorViewport.h"
#include "EditorViewportClient.h"
#include "SCommonEditorViewportToolbarBase.h"
#include "Misc/NotifyHook.h"

class ADialogueStandPositionTemplate;
class FAdvancedPreviewScene;
class UDialogueStandPositionTemplateAsset;

/**
 * 
 */
class AUTOGENSEQUENCER_EDITOR_API FStandTemplateEditor : public FAssetEditorToolkit, public FNotifyHook
{
	using Super = FAssetEditorToolkit;
public:

	FStandTemplateEditor();
	~FStandTemplateEditor();

	// Inherited via FAssetEditorToolkit
	FLinearColor GetWorldCentricTabColorScale() const override;
	FName GetToolkitFName() const override;
	FText GetBaseToolkitName() const override;
	FString GetWorldCentricTabPrefix() const override;
	void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	void SaveAsset_Execute() override;

	void InitStandTemplateEditor(const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost>& InToolkitHost, UDialogueStandPositionTemplateAsset* InAsset);
private:
	static const FName StandTemplateEditorDetailsTabId;
	static const FName StandTemplateEditorViewportTabId;

	TWeakObjectPtr<UDialogueStandPositionTemplateAsset> StandPositionTemplateAsset;

	TSharedPtr<IDetailsView> DetailsWidget;
	TSharedPtr<class SStandTemplateViewport> Viewport;

	TSharedRef<SDockTab> HandleTabManagerSpawnTabDetails(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> HandleTabManagerSpawnTabViewport(const FSpawnTabArgs& Args);
};

class FStandTemplateViewportClient : public FEditorViewportClient
{
	using Super = FEditorViewportClient;
public:
	FStandTemplateViewportClient(const TSharedRef<SStandTemplateViewport>& InViewport, const TSharedRef<FAdvancedPreviewScene>& InPreviewScene);

	TWeakPtr<class SStandTemplateViewport> ViewportPtr;
	FAdvancedPreviewScene* AdvancedPreviewScene;
public:
	TWeakObjectPtr<UDialogueStandPositionTemplateAsset> StandPositionTemplateAsset;
	TWeakObjectPtr<ADialogueStandPositionTemplate> PreviewTemplate;

	void InitStandTemplateViewportClient(UDialogueStandPositionTemplateAsset* InWarpper);
	void ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) override;
	void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	void DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas) override;
};

class SStandTemplateViewport : public SEditorViewport, public FGCObject, public ICommonEditorViewportToolbarInfoProvider
{
public:
	SLATE_BEGIN_ARGS(SStandTemplateViewport) {}
	SLATE_END_ARGS()

	/** The scene for this viewport. */
	TSharedPtr<FAdvancedPreviewScene> PreviewScene;

	void Construct(const FArguments& InArgs);

	SStandTemplateViewport();
	~SStandTemplateViewport();

	void AddReferencedObjects(FReferenceCollector& Collector) override;
	TSharedRef<class SEditorViewport> GetViewportWidget() override;
	TSharedPtr<FExtender> GetExtenders() const override;
	void OnFloatingButtonClicked() override;

	TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	TSharedPtr<SWidget> MakeViewportToolbar() override;

	FStandTemplateViewportClient* GetViewportClient() { return StandTemplateViewportClient.Get(); };
	//Shared ptr to the client
	TSharedPtr<FStandTemplateViewportClient> StandTemplateViewportClient;
};

