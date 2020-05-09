// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Kismet/BlueprintFunctionLibrary.h>
#include "DialogueAnimationUtils.generated.h"

class UCameraComponent;
class UAnimSequence;

/**
 *
 */
 UCLASS()
class AUTOGENSEQUENCER_API UDialogueAnimationUtils : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "动画")
    static TArray<FTransform> SampleSequence(const UAnimSequence* Sequence, float Time, const TArray<FName>& BoneNames);
};
