// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UCameraComponent;
class UAnimSequence;

/**
 *
 */
class XD_AUTOGENSEQUENCER_API FDialogueAnimationUtils
{
public:
	static float GetSimilarityWeights(const UAnimSequence* LHS, float LTime, const UAnimSequence* RHS, float RTime);
};
