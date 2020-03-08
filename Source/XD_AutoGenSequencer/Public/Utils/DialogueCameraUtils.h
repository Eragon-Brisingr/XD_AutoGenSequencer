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
	static bool ProjectWorldBoxBoundsToScreen(const FMinimalViewInfo& MinimalViewInfo, const FVector& Origin, const FVector& Extend, FBox2D& OutScreenBounds);
	static bool ProjectWorldBoxBoundsToScreen(UCameraComponent* CameraComponent, const FVector& Origin, const FVector& Extend, FBox2D& OutScreenBounds);
	static bool ProjectWorldBoxBoundsToScreen(const FSceneView* View, const FIntRect& CanvasRect, const FVector& Origin, const FVector& Extend, FBox2D& OutScreenBounds);

	static float ConvertWorldSphereRadiusToScreen(UCameraComponent* CameraComponent, const FVector& Origin, float Radius);
	static float ConvertWorldSphereRadiusToScreen(const FSceneView* View, const FVector& Origin, float Radius);

	// 功能函数
public:
	FORCEINLINE static FBox2D ClampBoundsInScreenRect(const FBox2D& ScreenBounds, const FIntRect& CanvasRect = FIntRect(0, 0, 1, 1))
	{
		return FBox2D(FVector2D(FMath::Max(ScreenBounds.Min.X, (float)CanvasRect.Min.X), FMath::Max(ScreenBounds.Min.Y, (float)CanvasRect.Min.Y)), 
						FVector2D(FMath::Min(ScreenBounds.Max.X, (float)CanvasRect.Max.X), FMath::Min(ScreenBounds.Max.Y, (float)CanvasRect.Max.Y)));
	}
	FORCEINLINE static float CalIntersectArea(const FBox2D& Box1, const FBox2D& Box2)
	{
		return (FMath::Min(Box1.Max.X, Box2.Max.X) - FMath::Max(Box1.Min.X, Box2.Min.X)) * (FMath::Min(Box1.Max.Y, Box2.Max.Y) - FMath::Max(Box1.Min.Y, Box2.Min.Y));
	};
private:
	static bool ProjectWorldBoxBoundsToScreen(const FVector& Origin, const FVector& Extend, FBox2D& OutScreenBounds, const TFunction<bool(const FVector&, FVector2D&)>& ProjectWorldToScreenFunction);
};
