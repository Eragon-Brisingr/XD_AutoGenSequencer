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
class UAutoGenDialogueAnimSetBase;

/**
 *
 */
UCLASS(Config = "AutoGenDialogueSettings", defaultconfig)
class AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueSettings : public UObject
{
	GENERATED_BODY()
public:
	UAutoGenDialogueSettings();
	
	UPROPERTY(EditAnywhere, Category = "Settings", Config, meta = (DisplayName = "对白角色设置类型"))
	TSoftClassPtr<UAutoGenDialogueCharacterSettings> DialogueCharacterSettingsType;
	static TSubclassOf<UAutoGenDialogueCharacterSettings> GetDialogueCharacterSettingsType();

	UPROPERTY(EditAnywhere, Category = "Settings", Config, meta = (DisplayName = "对白资源类型"))
	TSoftClassPtr<UDialogueSentence> DialogueSentenceType;
	static TSubclassOf<UDialogueSentence> GetDialogueSentenceType();
	
	UPROPERTY(EditAnywhere, Category = "Settings", Config, meta = (DisplayName = "对白动画集类型"))
	TSoftClassPtr<UAutoGenDialogueAnimSetBase> AutoGenDialogueAnimSetType;

	UPROPERTY(EditAnywhere, Category = "Settings", Config, meta = (DisplayName = "默认对白动画集"))
	TSoftObjectPtr<UAutoGenDialogueAnimSetBase> DefaultAutoGenDialogueAnimSet;

	static const UAutoGenDialogueSettings& Get() { return *GetDefault<UAutoGenDialogueSettings>(); }
};
