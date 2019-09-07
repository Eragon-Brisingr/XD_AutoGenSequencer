// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AutoGenDialogueSequenceFactory.generated.h"

/**
 * 
 */
UCLASS()
class XD_AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueSequenceFactory : public UFactory
{
	GENERATED_BODY()
public:
	UAutoGenDialogueSequenceFactory();

	UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

	FText GetDisplayName() const override;
	uint32 GetMenuCategories() const override;
};
