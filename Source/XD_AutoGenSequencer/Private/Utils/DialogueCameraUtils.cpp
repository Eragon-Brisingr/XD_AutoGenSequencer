// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/DialogueCameraUtils.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraTypes.h"
#include "SceneView.h"
#include <CanvasTypes.h>

void FDialogueCameraUtils::CameraTrackingTwoTargets(float CameraYawAngle, float FrontTargetRate, float BackTargetRate, FVector FrontTargetPosition, FVector BackTargetPosition, float Fov, FVector& CameraLocation, FRotator& CameraRotation, FVector& FocusCenterLocation)
{
	float Dis = FVector::Dist(BackTargetPosition, FrontTargetPosition);
	float SinCenterAngle, CosCenterAngle;
	FMath::SinCos(&SinCenterAngle, &CosCenterAngle, FMath::DegreesToRadians(FMath::Abs(CameraYawAngle)));
	float TanHalfFov = FMath::Tan(FMath::DegreesToRadians(Fov / 2.f));

	float BackTargetTimes = 1.f - BackTargetRate * 2;
	float FrontTargetTimes = 1.f - FrontTargetRate * 2; 

	check(BackTargetTimes != 0.f && FrontTargetTimes != 0.f && BackTargetTimes + FrontTargetTimes != 0.f);

	float BackTargetSum = SinCenterAngle / TanHalfFov / BackTargetTimes - CosCenterAngle;
	float FrontTargetSum = SinCenterAngle / TanHalfFov / FrontTargetTimes + CosCenterAngle;
	float FocusInterval = FrontTargetSum / (FrontTargetSum + BackTargetSum);
	float CenterInterval = Dis * FocusInterval * BackTargetSum;
	float BackWidthToEdge = (CenterInterval + Dis * FocusInterval * CosCenterAngle) * TanHalfFov;
	float FontWidthToEdge = (CenterInterval - Dis * (1.f - FocusInterval) * CosCenterAngle) * TanHalfFov;
	float BackTargetProjector = Dis * FocusInterval * SinCenterAngle;
	float FrontTargetProjector = Dis * (1.f - FocusInterval) * SinCenterAngle;

	FVector LookCenter = BackTargetPosition * (1.f - FocusInterval) + FrontTargetPosition * FocusInterval;
	CameraLocation = LookCenter + CenterInterval * FRotator(0.f, CameraYawAngle, 0.f).RotateVector((FrontTargetPosition - BackTargetPosition)).GetSafeNormal();
	CameraRotation = (LookCenter - CameraLocation).Rotation();
	FocusCenterLocation = LookCenter;
}

bool FDialogueCameraUtils::ProjectWorldToScreen(UCameraComponent* CameraComponent, const FVector& WorldLocation, FVector2D& OutScreenPosition)
{
	FMinimalViewInfo MinimalViewInfo;
	CameraComponent->GetCameraView(0.f, MinimalViewInfo);
	return ProjectWorldToScreen(MinimalViewInfo, WorldLocation, OutScreenPosition);
}

bool FDialogueCameraUtils::ProjectWorldToScreen(const FMinimalViewInfo& MinimalViewInfo, const FVector& WorldLocation, FVector2D& OutScreenPosition)
{
	const FMatrix ProjectionMatrix = MinimalViewInfo.CalculateProjectionMatrix();
	if (FSceneView::ProjectWorldToScreen(WorldLocation, FIntRect(0, 0, 1, 1), 
		FLookAtMatrix(MinimalViewInfo.Location, MinimalViewInfo.Location + MinimalViewInfo.Rotation.Vector(), FVector::UpVector) * ProjectionMatrix, OutScreenPosition))
	{
		bool bIsInScreen = OutScreenPosition.X > 0.f && OutScreenPosition.X < 1.f && OutScreenPosition.Y > 0.f && OutScreenPosition.Y < 1.f;
		return bIsInScreen;
	}
	return false;
}

bool FDialogueCameraUtils::ProjectWorldToScreen(const FSceneView* View, const FIntRect& CanvasRect, const FVector& WorldLocation, FVector2D& OutScreenPosition)
{
	return View->ProjectWorldToScreen(WorldLocation, CanvasRect, View->ViewMatrices.GetViewProjectionMatrix(), OutScreenPosition);
}

bool FDialogueCameraUtils::ProjectWorldBoxBoundsToScreen(UCameraComponent* CameraComponent, const FVector& Origin, const FVector& Extend, FBox2D& OutScreenBounds)
{
	FMinimalViewInfo MinimalViewInfo;
	CameraComponent->GetCameraView(0.f, MinimalViewInfo);

	return ProjectWorldBoxBoundsToScreen(Origin, Extend, OutScreenBounds, [&](const FVector& WorldPosition, FVector2D& ScreenPosition)
		{
			return ProjectWorldToScreen(MinimalViewInfo, WorldPosition, ScreenPosition);
		});
}

bool FDialogueCameraUtils::ProjectWorldBoxBoundsToScreen(const FSceneView* View, const FIntRect& CanvasRect, const FVector& Origin, const FVector& Extend, FBox2D& OutScreenBounds)
{
	return ProjectWorldBoxBoundsToScreen(Origin, Extend, OutScreenBounds, [&](const FVector& WorldPosition, FVector2D& ScreenPosition)
		{
			return ProjectWorldToScreen(View, CanvasRect, WorldPosition, ScreenPosition);
		});
}

bool FDialogueCameraUtils::ProjectWorldBoxBoundsToScreen(const FVector& Origin, const FVector& Extend, FBox2D& OutScreenBounds, const TFunction<bool(const FVector&, FVector2D&)>& ProjectWorldToScreenFunction)
{
	TArray<FVector> Points;
	// calculate 3D corner Points of bounding box
	Points.Add(Origin + FVector(Extend.X, Extend.Y, Extend.Z));
	Points.Add(Origin + FVector(-Extend.X, Extend.Y, Extend.Z));
	Points.Add(Origin + FVector(Extend.X, -Extend.Y, Extend.Z));
	Points.Add(Origin + FVector(-Extend.X, -Extend.Y, Extend.Z));
	Points.Add(Origin + FVector(Extend.X, Extend.Y, -Extend.Z));
	Points.Add(Origin + FVector(-Extend.X, Extend.Y, -Extend.Z));
	Points.Add(Origin + FVector(Extend.X, -Extend.Y, -Extend.Z));
	Points.Add(Origin + FVector(-Extend.X, -Extend.Y, -Extend.Z));

	FVector2D MinScreenWidgets(TNumericLimits<float>().Max(), TNumericLimits<float>().Max());
	FVector2D MaxScreenWidgets(TNumericLimits<float>().Lowest(), TNumericLimits<float>().Lowest());
	bool IsCompletelyInView = true;
	for (const FVector& Point : Points) {
		FVector2D ScreenPosition(0, 0);
		IsCompletelyInView &= ProjectWorldToScreenFunction(Point, ScreenPosition);
		MaxScreenWidgets.X = FMath::Max(ScreenPosition.X, MaxScreenWidgets.X);
		MaxScreenWidgets.Y = FMath::Max(ScreenPosition.Y, MaxScreenWidgets.Y);
		MinScreenWidgets.X = FMath::Min(ScreenPosition.X, MinScreenWidgets.X);
		MinScreenWidgets.Y = FMath::Min(ScreenPosition.Y, MinScreenWidgets.Y);
	}
	OutScreenBounds = FBox2D(MinScreenWidgets, MaxScreenWidgets);

	return IsCompletelyInView;
}

float FDialogueCameraUtils::ConvertWorldSphereRadiusToScreen(UCameraComponent* CameraComponent, const FVector& Origin, float Radius)
{
	float DistanceToObject = FVector(Origin - CameraComponent->GetComponentLocation()).Size();

	/* Get Projected Screen Radius */
	float ScreenRadius = FMath::Atan(Radius / DistanceToObject) * FMath::DegreesToRadians(CameraComponent->FieldOfView);
	return ScreenRadius;
}

float FDialogueCameraUtils::ConvertWorldSphereRadiusToScreen(const FSceneView* View, const FVector& Origin, float Radius)
{
	float DistanceToObject = FVector(Origin - View->ViewLocation).Size();

	/* Get Projected Screen Radius */
	float ScreenRadius = FMath::Atan(Radius / DistanceToObject) * FMath::DegreesToRadians(View->FOV);
	return ScreenRadius;
}
