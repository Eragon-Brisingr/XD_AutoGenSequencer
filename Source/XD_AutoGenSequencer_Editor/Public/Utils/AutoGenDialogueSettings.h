// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <UObject/NoExportTypes.h>
#include "AutoGenDialogueSettings.generated.h"

class UDialogueSentence;
class UDialogueSentenceSection;
class UAutoGenDialogueAnimSetBase;
class UAutoGenDialogueCameraSet;
class UGenDialogueSequenceConfigBase;
class UAutoGenDialogueCharacterSettings;

/**
 *
 */
UCLASS(Config = "AutoGenDialogueSettings", defaultconfig)
class XD_AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueSettings : public UObject
{
	GENERATED_BODY()
public:
	UAutoGenDialogueSettings();

	UPROPERTY(EditAnywhere, Category = "Settings", Config)
	TSoftClassPtr<UDialogueSentence> DialogueSentenceType;
	static TSubclassOf<UDialogueSentence> GetDialogueSentenceType();

	UPROPERTY(EditAnywhere, Category = "Settings", Config)
	TSoftObjectPtr<UAutoGenDialogueAnimSetBase> DefaultAutoGenDialogueAnimSet;

	UPROPERTY(EditAnywhere, Category = "Settings", Config)
	TSoftObjectPtr<UAutoGenDialogueCameraSet> DefaultAutoGenDialogueCameraSet;

	UPROPERTY(EditAnywhere, Category = "Settings", Config)
	TSoftClassPtr<UAutoGenDialogueCharacterSettings> DefaultDialogueCharacterSettingsType;
	static TSubclassOf<UAutoGenDialogueCharacterSettings> GetDefaultDialogueCharacterSettingsType();

	static const UAutoGenDialogueSettings& Get() { return *GetDefault<UAutoGenDialogueSettings>(); }
};
