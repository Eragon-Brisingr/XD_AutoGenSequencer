// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <GameFramework/Info.h>
#include <Factories/Factory.h>
#include <AssetTypeActions_Base.h>
#include <ThumbnailRendering/DefaultSizedThumbnailRenderer.h>
#include "DialogueStandPositionTemplate.generated.h"

class ACharacter;
class UChildActorComponent;
class UAutoGenDialogueCameraTemplateAsset;

USTRUCT()
struct FDialogueStandPosition
{
	GENERATED_BODY()
public:
	FDialogueStandPosition() = default;
	FDialogueStandPosition(const FName& StandName, const TSubclassOf<ACharacter>& PreviewCharacter, const FTransform& StandPosition)
		:StandPosition(StandPosition), StandName(StandName), PreviewCharacter(PreviewCharacter)
	{}

	UPROPERTY(EditAnywhere, meta = (MakeEditWidget = true), meta = (DisplayName = "模板站立位置"))
	FTransform StandPosition;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "站位角色名称"))
	FName StandName;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "站位预览角色"))
	TSubclassOf<ACharacter> PreviewCharacter;

	UPROPERTY(Transient)
	UChildActorComponent* PreviewCharacterInstance = nullptr;
};

UCLASS(NotBlueprintable, hidedropdown, hidecategories = (Input, Movement, Collision, Rendering, Replication, Actor, LOD, Cooking, Tick))
class AUTOGENSEQUENCER_EDITOR_API ADialogueStandPositionTemplate : public AInfo
{
	GENERATED_BODY()
public:
	ADialogueStandPositionTemplate();

	UPROPERTY(EditAnywhere, Category = "站位模板", meta = (DisplayName = "预览用角色"))
	TSubclassOf<ACharacter> PreviewCharacter;

	UPROPERTY(EditAnywhere, Category = "站位模板", meta = (DisplayName = "站位配置"))
	TArray<FDialogueStandPosition> StandPositions;

	UPROPERTY(EditAnywhere, Category = "站位模板", meta = (DisplayName = "镜头模板集"))
	UAutoGenDialogueCameraSet* AutoGenDialogueCameraSet;

	void PreEditChange(FProperty* PropertyThatWillChange) override;
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void OnConstruction(const FTransform& Transform) override;

	UChildActorComponent* CreateChildActorComponent();
	void CreateAllTemplatePreviewCharacter();
	void UpdateToStandLocation();

	UPROPERTY(Transient)
	TArray<UChildActorComponent*> PreviewCharacters;

	DECLARE_DELEGATE(FOnInstanceChanged);
	FOnInstanceChanged OnInstanceChanged;
};

UCLASS()
class AUTOGENSEQUENCER_EDITOR_API UDialogueStandPositionTemplateAsset : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Instanced)
	ADialogueStandPositionTemplate* Template;
};

UCLASS()
class AUTOGENSEQUENCER_EDITOR_API UDialogueStandPositionTemplateFactory : public UFactory
{
	GENERATED_BODY()
public:
	UDialogueStandPositionTemplateFactory();

	UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	FText GetDisplayName() const override;
};

class FAssetTypeActions_DialogueStandPositionTemplate : public FAssetTypeActions_Base
{
	using Super = FAssetTypeActions_Base;

	// Inherited via FAssetTypeActions_Base
	FText GetName() const override;
	UClass* GetSupportedClass() const override;
	FColor GetTypeColor() const override;
	uint32 GetCategories() override;
	void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor) override;
};

UCLASS()
class UDialogueStandPositionTemplateThumbnailRenderer : public UDefaultSizedThumbnailRenderer
{
	GENERATED_BODY()
public:
	void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily) override;
	void BeginDestroy() override;
	bool AllowsRealtimeThumbnails(UObject* Object) const override { return false; }
private:
	class FAdvancedPreviewScene* ThumbnailScene = nullptr;
};
