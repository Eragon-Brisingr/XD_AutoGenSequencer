// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Factories/Factory.h"
#include "AutoGenDialogueAnimSet.generated.h"

class UAnimSequence;

/**
 * 
 */
UCLASS()
class XD_AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueAnimSet : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TArray<UAnimSequence*> TalkAnims;

	UPROPERTY(EditAnywhere)
	TArray<UAnimSequence*> IdleAnims;
};

UCLASS()
class XD_AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueAnimSetFactory : public UFactory
{
	GENERATED_BODY()
public:
	UAutoGenDialogueAnimSetFactory();

	UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;

	FText GetDisplayName() const override;
	uint32 GetMenuCategories() const override;
};

