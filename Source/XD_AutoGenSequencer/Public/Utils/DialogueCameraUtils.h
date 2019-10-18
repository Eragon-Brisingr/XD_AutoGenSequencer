// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UCameraComponent;
class FSceneView;

/**
 * 
 */
class XD_AUTOGENSEQUENCER_API FDialogueCameraUtils
{
public:
	static void CameraTrackingTwoTargets(float CameraYawAngle, float FrontTargetRate, float BackTargetRate, FVector FrontTargetPosition, FVector BackTargetPosition, float Fov, FVector& CameraLocation, FRotator& CameraRotation, FVector& FocusCenterLocation);

	// 屏幕投影
public:
	static bool ProjectWorldToScreen(UCameraComponent* CameraComponent, const FVector& WorldLocation, FVector2D& OutScreenPosition);
	static bool ProjectWorldToScreen(const FMinimalViewInfo& MinimalViewInfo, const FVector& WorldLocation, FVector2D& OutScreenPosition);
	static bool ProjectWorldToScreen(const FSceneView* View, const FIntRect& CanvasRect, const FVector& WorldLocation, FVector2D& OutScreenPosition);

	// 计算包围盒到屏幕空间的权重包围盒
	static bool ProjectWorldBoxBoundsToScreen(UCameraComponent* CameraComponent, const FVector& Origin, const FVector& Extend, FBox2D& OutScreenBounds);
	static bool ProjectWorldBoxBoundsToScreen(const FSceneView* View, const FIntRect& CanvasRect, const FVector& Origin, const FVector& Extend, FBox2D& OutScreenBounds);

	static float ConvertWorldSphereRadiusToScreen(UCameraComponent* CameraComponent, const FVector& Origin, float Radius);
	static float ConvertWorldSphereRadiusToScreen(const FSceneView* View, const FVector& Origin, float Radius);
private:
	static bool ProjectWorldBoxBoundsToScreen(const FVector& Origin, const FVector& Extend, FBox2D& OutScreenBounds, const TFunction<bool(const FVector&, FVector2D&)>& ProjectWorldToScreenFunction);
};
