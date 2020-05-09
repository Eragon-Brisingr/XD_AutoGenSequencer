// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Factories/Factory.h>
#include <ILevelSequenceMetaData.h>
#include "AutoGenDialogueSequenceFactory.generated.h"

class UGenDialogueSequenceConfigBase;
class ULevelSequence;

/**
 * 
 */
UCLASS()
class AUTOGENSEQUENCER_EDITOR_API UGenDialogueSequenceConfigContainer : public UObject, public ILevelSequenceMetaData
{
	GENERATED_BODY()
public:
	UPROPERTY()
	UGenDialogueSequenceConfigBase* GenDialogueSequenceConfig;

public:

	// TODO：补全资源标签

	/**
	 * Extend the default ULevelSequence asset registry tags
	 */
	void ExtendAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override {}

	/**
	 * Extend the default ULevelSequence asset registry tag meta-data
	 */
	void ExtendAssetRegistryTagMetaData(TMap<FName, FAssetRegistryTagMetadata>& OutMetadata) const override {}
};

UCLASS()
class AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueSequenceFactory : public UFactory
{
	GENERATED_BODY()
public:
	UAutoGenDialogueSequenceFactory();
	
	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	TSubclassOf<UGenDialogueSequenceConfigBase> AutoGenDialogueSequenceConfigClass;

	UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;

	bool ConfigureProperties() override;

	static void AddGenDialogueSequenceConfig(ULevelSequence* LevelSequence, TSubclassOf<UGenDialogueSequenceConfigBase> AutoGenDialogueSequenceConfigClass);
	static bool ShowPickConfigClassViewer(UClass*& ChosenClass);

	FText GetDisplayName() const override;
	uint32 GetMenuCategories() const override;
};
