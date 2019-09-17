// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AutoGenDialogueSequenceFactory.generated.h"

class UAutoGenDialogueSequenceConfig;
class ULevelSequence;

/**
 * 
 */
UCLASS()
class XD_AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueSequenceFactory : public UFactory
{
	GENERATED_BODY()
public:
	UAutoGenDialogueSequenceFactory();
	
	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	TSubclassOf<UGenDialogueSequenceConfigBase> AutoGenDialogueSequenceConfigClass;

	UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;

	bool ConfigureProperties() override;

	static void AddAutoGenDialogueSystemData(ULevelSequence* LevelSequence, TSubclassOf<UGenDialogueSequenceConfigBase> AutoGenDialogueSequenceConfigClass);
	static bool ShowPickConfigClassViewer(UClass*& ChosenClass);

	FText GetDisplayName() const override;
	uint32 GetMenuCategories() const override;
};
