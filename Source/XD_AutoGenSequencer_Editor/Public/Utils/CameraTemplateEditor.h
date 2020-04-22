// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "SEditorViewport.h"
#include "EditorViewportClient.h"
#include "SCommonEditorViewportToolbarBase.h"
#include "Misc/NotifyHook.h"

class UAutoGenDialogueCameraTemplateAsset;
class FAdvancedPreviewScene;
class AAutoGenDialogueCameraTemplate;

/**
 * 
 */
class XD_AUTOGENSEQUENCER_EDITOR_API FCameraTemplateEditor : public FAssetEditorToolkit, public FNotifyHook
{
	using Super = FAssetEditorToolkit;
public:
	FCameraTemplateEditor();
	~FCameraTemplateEditor();

	// Inherited via FAssetEditorToolkit
	FLinearColor GetWorldCentricTabColorScale() const override;
	FName GetToolkitFName() const override;
	FText GetBaseToolkitName() const override;
	FString GetWorldCentricTabPrefix() const override;
	void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	void SaveAsset_Execute() override;

	void InitCameraTemplateEditor(const EToolkitMode::Type InMode, const TSharedPtr<class IToolkitHost>& InToolkitHost, UAutoGenDialogueCameraTemplateAsset* InAsset);
private:
	static const FName CameraTemplateEditorDetailsTabId;
	static const FName CameraTemplateEditorViewportTabId;

	TWeakObjectPtr<UAutoGenDialogueCameraTemplateAsset> CameraTemplateAsset;

	TSharedPtr<SGraphEditor> EdGraphEditor;
	TSharedPtr<IDetailsView> DetailsWidget;
	TSharedPtr<class SCameraTemplateViewport> Viewport;

	TSharedRef<SDockTab> HandleTabManagerSpawnTabDetails(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> HandleTabManagerSpawnTabViewport(const FSpawnTabArgs& Args);
};

class FCameraTemplateViewportClient : public FEditorViewportClient
{
	using Super = FEditorViewportClient;
public:
	FCameraTemplateViewportClient(const TSharedRef<SCameraTemplateViewport>& InThumbnailViewport, const TSharedRef<FAdvancedPreviewScene>& InPreviewScene);

	TWeakPtr<class SCameraTemplateViewport> ViewportPtr;
	FAdvancedPreviewScene* AdvancedPreviewScene;
public:
	TWeakObjectPtr<UAutoGenDialogueCameraTemplateAsset> CameraTemplateAsset;
	AAutoGenDialogueCameraTemplate* PreviewCameraTemplate = nullptr;

	void InitCameraTemplateViewportClient(UAutoGenDialogueCameraTemplateAsset* InAsset);
	void ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) override;
	void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
	void DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas) override;
	void Tick(float DeltaSeconds) override;

	bool GetActiveSafeFrame(float& OutAspectRatio) const override;
	ELevelViewportType GetViewportType() const override;
	void OverridePostProcessSettings(FSceneView& View) override;
};

class SCameraTemplateViewport : public SEditorViewport, public FGCObject, public ICommonEditorViewportToolbarInfoProvider
{
public:
	SLATE_BEGIN_ARGS(SCameraTemplateViewport) {}
	SLATE_END_ARGS()

	/** The scene for this viewport. */
	TSharedPtr<FAdvancedPreviewScene> PreviewScene;

	SCameraTemplateViewport();
	~SCameraTemplateViewport();
	
	void Construct(const FArguments& InArgs);

	void AddReferencedObjects(FReferenceCollector& Collector) override {}
	TSharedRef<class SEditorViewport> GetViewportWidget() override;
	TSharedPtr<FExtender> GetExtenders() const override;
	void OnFloatingButtonClicked() override;

	TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;

	FCameraTemplateViewportClient* GetViewportClient() { return CameraTemplateViewportClient.Get(); };
	//Shared ptr to the client
	TSharedPtr<FCameraTemplateViewportClient> CameraTemplateViewportClient;
};
