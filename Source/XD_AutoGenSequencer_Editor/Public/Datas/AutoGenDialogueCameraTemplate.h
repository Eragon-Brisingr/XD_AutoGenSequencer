// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <GameFramework/Actor.h>
#include <MovieSceneObjectBindingID.h>
#include "Factories/Factory.h"
#include "Datas/AutoGenDialogueType.h"
#include "AutoGenDialogueCameraTemplate.generated.h"

class UChildActorComponent;
class ADialogueStandPositionTemplate;
class UCineCameraComponent;
class UMovieScene;
class ACharacter;
class ACineCameraActor;
struct FGenDialogueCharacterData;

UCLASS(abstract, NotBlueprintable, NotBlueprintType, hidedropdown, hidecategories = (Input, Movement, Collision, Rendering, Replication, Actor, LOD, Cooking))
class XD_AUTOGENSEQUENCER_EDITOR_API AAutoGenDialogueCameraTemplate : public AActor
{
	GENERATED_BODY()
public:
	AAutoGenDialogueCameraTemplate();

	void PostInitializeComponents() override;
	void Destroyed() override;
	void PreEditChange(FProperty* PropertyThatWillChange) override;
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	UPROPERTY()
	USceneComponent* TemplateRoot;
	UPROPERTY(VisibleAnywhere, Transient, Category = "镜头模板")
	UCineCameraComponent* CineCameraComponent;
	UPROPERTY()
	ACineCameraActor* CineCameraActorTemplate;

	virtual void UpdateCameraTransform() {}
public:
	struct FCameraWeightsData
	{
		FCameraWeightsData() = default;
		const AAutoGenDialogueCameraTemplate* CameraTemplate = nullptr;
		FVector CameraLocation;
		FRotator CameraRotation;
		float Weights;
	};

	struct FDialogueCameraCutData
	{
		TRange<FFrameNumber> CameraCutRange;

		struct FDialogueData
		{
			ACharacter* Speaker;
			TArray<ACharacter*> Targets;
			UDialogueSentence* DialogueSentence;
			TRange<FFrameNumber> DialogueRange;
		};
		TArray<FDialogueData> DialogueDatas;
	};

	// 用于评估该镜头所处的对话环境中的分数
	virtual TOptional<AAutoGenDialogueCameraTemplate::FCameraWeightsData> EvaluateCameraTemplate(ACharacter* LookTarget, const TArray<ACharacter*>& Others, const TMap<ACharacter*, FGenDialogueCharacterData>& DialogueCharacterDataMap, float DialogueProgress) const { return FCameraWeightsData(); }
	// 用于生成该镜头对应的轨道
	virtual void GenerateCameraTrackData(ACharacter* LookTarget, const TArray<ACharacter*>& Others, UMovieScene& MovieScene, FGuid CineCameraComponentGuid, const TMap<ACharacter*, FGenDialogueCharacterData>& DialogueCharacterDataMap, const TArray<FDialogueCameraCutData>& DialogueCameraCutDatas) const {}
private:
	UPROPERTY(Transient)
	ACineCameraActor* CineCameraActor;
};

UCLASS()
class XD_AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueCameraTemplateAsset : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Instanced)
	AAutoGenDialogueCameraTemplate* Template;
};

UCLASS()
class XD_AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueCameraTemplateFactory : public UFactory
{
	GENERATED_BODY()
public:
	UAutoGenDialogueCameraTemplateFactory();

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	TSubclassOf<AAutoGenDialogueCameraTemplate> CameraTemplateClass;

	UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;

	bool ConfigureProperties() override;
	FText GetDisplayName() const override;
};

class FAssetTypeActions_AutoGenDialogueCameraTemplate : public FAssetTypeActions_Base
{
	using Super = FAssetTypeActions_Base;

	// Inherited via FAssetTypeActions_Base
	FText GetName() const override;
	UClass* GetSupportedClass() const override;
	FColor GetTypeColor() const override;
	uint32 GetCategories() override;
	void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor) override;
};
