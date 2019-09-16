// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AutoGenDialogueRuntimeSettings.generated.h"

/**
 *
 */
UCLASS(Config = "AutoGenDialogueSettings", defaultconfig)
class XD_AUTOGENSEQUENCER_API UAutoGenDialogueRuntimeSettings : public UObject
{
	GENERATED_BODY()
public:
	UAutoGenDialogueRuntimeSettings();

	UPROPERTY(EditAnywhere, Category = "Settings", Config)
	TEnumAsByte<ETraceTypeQuery> CameraEvaluateTraceChannel;
};
