// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <UObject/NoExportTypes.h>
#include "Factories/Factory.h"
#include "AssetTypeActions_Base.h"
#include "AutoGenDialogueAnimSet.generated.h"

class UAnimSequence;

/**
 * 
 */
UCLASS(abstract)
class XD_AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueAnimSetBase : public UObject
{
	GENERATED_BODY()
public:
	virtual bool IsAnimSetValid(TArray<FText>& ErrorMessages) const;
};

UCLASS(meta = (DisplayName = "对话动作集"))
class XD_AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueAnimSet : public UAutoGenDialogueAnimSetBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "对白动画集"))
	TArray<UAnimSequence*> TalkAnims;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "站立动画集"))
	TArray<UAnimSequence*> IdleAnims;

	bool IsAnimSetValid(TArray<FText>& ErrorMessages) const override;
};

UCLASS()
class XD_AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueAnimSetFactory : public UFactory
{
	GENERATED_BODY()
public:
	UAutoGenDialogueAnimSetFactory();

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	TSubclassOf<UAutoGenDialogueAnimSetBase> AutoGenDialogueAnimSetClass;

	UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;

	bool ConfigureProperties() override;
	FText GetDisplayName() const override;
	uint32 GetMenuCategories() const override;
};

class FAssetTypeActions_AutoGenDialogueAnimSet : public FAssetTypeActions_Base
{
	using Super = FAssetTypeActions_Base;

	// Inherited via FAssetTypeActions_Base
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override;
	virtual uint32 GetCategories() override;
};