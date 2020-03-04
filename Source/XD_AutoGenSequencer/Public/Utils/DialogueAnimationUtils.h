// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <UObject/NoExportTypes.h>
#include "DialogueAnimationUtils.generated.h"

class UCameraComponent;
class UAnimSequence;

/**
 *
 */
 UCLASS()
class XD_AUTOGENSEQUENCER_API UDialogueAnimationUtils : public UObject
{
    GENERATED_BODY()
public:
	static float GetSimilarityWeights(const UAnimSequence* LHS, float LTime, const UAnimSequence* RHS, float RTime);

    UFUNCTION(BlueprintCallable, Category = "动画")
    static TArray<FTransform> SampleSequence(const UAnimSequence* Sequence, float Time, const TArray<FName>& BoneNames);
};
