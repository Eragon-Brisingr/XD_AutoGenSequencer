// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class XD_AUTOGENSEQUENCER_API FDialogueCameraUtils
{
public:
	static void CameraTrackingTwoTargets(float CameraYawAngle, float FrontTargetRate, float BackTargetRate, FVector FrontTargetPosition, FVector BackTargetPosition, float Fov, FVector& CameraLocation, FRotator& CameraRotation, FVector& FocusCenterLocation);
};
