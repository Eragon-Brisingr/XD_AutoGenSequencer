// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UCameraComponent;

/**
 * 
 */
class XD_AUTOGENSEQUENCER_API FDialogueCameraUtils
{
public:
	static void CameraTrackingTwoTargets(float CameraYawAngle, float FrontTargetRate, float BackTargetRate, FVector FrontTargetPosition, FVector BackTargetPosition, float Fov, FVector& CameraLocation, FRotator& CameraRotation, FVector& FocusCenterLocation);

	// 屏幕投影
public:
	static bool WorldToScreenWidgets(UCameraComponent* CameraComponent, const FVector& WorldLocation, FVector2D& OutScreenWidgets);
	static bool WorldToScreenWidgets(const FMinimalViewInfo& MinimalViewInfo, const FVector& WorldLocation, FVector2D& OutScreenWidgets);

	// 计算包围盒到屏幕空间的权重包围盒
	static bool WorldBoundsToScreenWidgets(UCameraComponent* CameraComponent, const FVector& Origin, const FVector& Extend, FBox2D& OutScreenWidgetsBounds);
};
