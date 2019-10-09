// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueCameraUtils.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraTypes.h"
#include "SceneView.h"

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

bool FDialogueCameraUtils::WorldToScreenWidgets(UCameraComponent* CameraComponent, const FVector& WorldLocation, FVector2D& OutScreenWidgets)
{
	FMinimalViewInfo MinimalViewInfo;
	CameraComponent->GetCameraView(0.f, MinimalViewInfo);
	return WorldToScreenWidgets(MinimalViewInfo, WorldLocation, OutScreenWidgets);
}

bool FDialogueCameraUtils::WorldToScreenWidgets(const FMinimalViewInfo& MinimalViewInfo, const FVector& WorldLocation, FVector2D& OutScreenWidgets)
{
	const FMatrix ProjectionMatrix = MinimalViewInfo.CalculateProjectionMatrix();
	if (FSceneView::ProjectWorldToScreen(WorldLocation, FIntRect(0, 0, 1, 1), 
		FLookAtMatrix(MinimalViewInfo.Location, MinimalViewInfo.Location + MinimalViewInfo.Rotation.Vector(), FVector::UpVector) * ProjectionMatrix, OutScreenWidgets))
	{
		bool bIsInScreen = OutScreenWidgets.X > 0.f && OutScreenWidgets.X < 1.f && OutScreenWidgets.Y > 0.f && OutScreenWidgets.Y < 1.f;
		return bIsInScreen;
	}
	return false;
}

bool FDialogueCameraUtils::WorldBoundsToScreenWidgets(UCameraComponent* CameraComponent, const FVector& Origin, const FVector& Extend, FBox2D& OutScreenWidgetsBounds)
{
	FMinimalViewInfo MinimalViewInfo;
	CameraComponent->GetCameraView(0.f, MinimalViewInfo);

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

	FVector2D MinScreenWidgets(1.f, 1.f);
	FVector2D MaxScreenWidgets(0, 0);
	bool IsCompletelyInView = true;
	for (const FVector& Point : Points) {
		FVector2D ScreenWidgets(0, 0);
		IsCompletelyInView &= WorldToScreenWidgets(MinimalViewInfo, Point, ScreenWidgets);
		MaxScreenWidgets.X = FMath::Max(ScreenWidgets.X, MaxScreenWidgets.X);
		MaxScreenWidgets.Y = FMath::Max(ScreenWidgets.Y, MaxScreenWidgets.Y);
		MinScreenWidgets.X = FMath::Min(ScreenWidgets.X, MinScreenWidgets.X);
		MinScreenWidgets.Y = FMath::Min(ScreenWidgets.Y, MinScreenWidgets.Y);
	}
	OutScreenWidgetsBounds = FBox2D(MinScreenWidgets, MaxScreenWidgets);

	return IsCompletelyInView;
}
