// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AssetTypeActions_Base.h"
#include "DialogueSentenceFactory.generated.h"

/**
 * 
 */
UCLASS()
class AUTOGENSEQUENCER_EDITOR_API UDialogueSentenceFactory : public UFactory
{
	GENERATED_BODY()
public:
	UDialogueSentenceFactory();

	UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;

	FText GetDisplayName() const override;
	uint32 GetMenuCategories() const override;
};

class FAssetTypeActions_DialogueSentence : public FAssetTypeActions_Base
{
	using Super = FAssetTypeActions_Base;

	// Inherited via FAssetTypeActions_Base
	FText GetName() const override;
	UClass* GetSupportedClass() const override;
	FColor GetTypeColor() const override;
	uint32 GetCategories() override;
	FText GetDisplayNameFromAssetData(const FAssetData& AssetData) const override;
};
