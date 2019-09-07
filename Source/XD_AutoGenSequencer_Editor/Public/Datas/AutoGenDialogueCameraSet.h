// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SubclassOf.h"
#include "Factories/Factory.h"
#include "AutoGenDialogueCameraSet.generated.h"

class AAutoGenDialogueCameraTemplate;

/**
 * 
 */
USTRUCT()
struct XD_AUTOGENSEQUENCER_EDITOR_API FAutoGenDialogueCameraConfig
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AAutoGenDialogueCameraTemplate> CameraTemplate;
	UPROPERTY(EditAnywhere)
	float Weights = 1.f;
};

UCLASS()
class XD_AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueCameraSet : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TArray<FAutoGenDialogueCameraConfig> CameraTemplates;

	bool IsValid() const;
};

UCLASS()
class XD_AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueCameraSetFactory : public UFactory
{
	GENERATED_BODY()
public:
	UAutoGenDialogueCameraSetFactory();

	UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;

	FText GetDisplayName() const override;
	uint32 GetMenuCategories() const override;
};

