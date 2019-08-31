// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AutoGenDialogueAnimSet.generated.h"

class UAnimSequence;

/**
 * 
 */
UCLASS()
class XD_AUTOGENSEQUENCER_API UAutoGenDialogueAnimSet : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	TArray<UAnimSequence*> Anims;
};
