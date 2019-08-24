// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "DialogueSequenceConfigFactory.generated.h"

/**
 * 
 */
UCLASS()
class XD_AUTOGENSEQUENCER_EDITOR_API UDialogueSequenceConfigFactory : public UFactory
{
	GENERATED_BODY()
public:
	UDialogueSequenceConfigFactory();

	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	FText GetDisplayName() const override;
	uint32 GetMenuCategories() const override;
};
