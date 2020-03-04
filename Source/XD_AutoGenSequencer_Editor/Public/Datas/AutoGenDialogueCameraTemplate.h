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
struct FGenDialogueCharacterData;

UCLASS(Transient, abstract, NotBlueprintable, NotBlueprintType, hidedropdown, hidecategories = (Input, Movement, Collision, Rendering, Replication, Actor, LOD, Cooking))
class XD_AUTOGENSEQUENCER_EDITOR_API AAutoGenDialogueCameraTemplate : public AActor
{
	GENERATED_BODY()

public:
	AAutoGenDialogueCameraTemplate();

	void OnConstruction(const FTransform& Transform) override;

	void PreEditChange(UProperty* PropertyThatWillChange) override;
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	UPROPERTY(EditAnywhere, Category = "镜头模板", meta = (DisplayName = "站位模板"))
	TSubclassOf<ADialogueStandPositionTemplate> StandTemplate;

	UPROPERTY()
	UChildActorComponent* StandTemplatePreview;

	UPROPERTY(VisibleAnywhere, Transient, Category = "镜头模板")
	UCineCameraComponent* CineCameraComponent;
	UPROPERTY()
	UChildActorComponent* CineCamera;

	// 现在只是将摄像机的位置赋给了视口，Fov等数据并没有同步，只是用于简单预览
	UPROPERTY(EditDefaultsOnly, Category = "镜头模板", meta = (DisplayName = "自动预览摄像机位置"))
	uint8 bActiveCameraViewport : 1;
public:
	struct FCameraWeightsData
	{
		const AAutoGenDialogueCameraTemplate* CameraTemplate;
		FVector CameraLocation;
		FRotator CameraRotation;
		float Weights;
		bool IsValid() { return CameraTemplate ? true : false; }
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
	virtual FCameraWeightsData EvaluateCameraTemplate(ACharacter* LookTarget, const TArray<ACharacter*>& Others, const TMap<ACharacter*, FGenDialogueCharacterData>& DialogueCharacterDataMap, float DialogueProgress) const { return FCameraWeightsData(); }
	// 用于生成该镜头对应的轨道
	virtual void GenerateCameraTrackData(ACharacter* LookTarget, const TArray<ACharacter*>& Others, UMovieScene& MovieScene, FGuid CineCameraComponentGuid, const TMap<ACharacter*, FGenDialogueCharacterData>& DialogueCharacterDataMap, const TArray<FDialogueCameraCutData>& DialogueCameraCutDatas) const {}
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
	uint32 GetMenuCategories() const override;
};

