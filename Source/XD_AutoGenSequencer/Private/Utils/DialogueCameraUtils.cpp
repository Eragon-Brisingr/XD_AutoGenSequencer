// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueCameraUtils.h"

void FDialogueCameraUtils::CameraTrackingTwoTargets(float CameraYawAngle, float FrontTargetRate, float BackTargetRate, FVector FrontTargetPosition, FVector BackTargetPosition, float Fov, float Aspect, FVector& CameraPosition, FRotator& CameraRotation)
{
	float Dis = FVector::Dist(BackTargetPosition, FrontTargetPosition);
	float SinCenterAngle, CosCenterAngle;
	FMath::SinCos(&SinCenterAngle, &CosCenterAngle, FMath::DegreesToRadians(CameraYawAngle));
	float TanHalfFov = FMath::Tan(FMath::DegreesToRadians(Fov / 2.f));

	float BackTargetTimes = 1.f - FrontTargetRate * 2;
	float FrontTargetTimes = 1.f - BackTargetRate * 2;

	check(BackTargetTimes != 0 && FrontTargetTimes != 0 && BackTargetTimes + FrontTargetTimes != 0);

	float HalfWidthDivideCenterDis = TanHalfFov * Aspect;
	float BackTargetSum = SinCenterAngle / HalfWidthDivideCenterDis / BackTargetTimes - CosCenterAngle;
	float FrontTargetSum = SinCenterAngle / HalfWidthDivideCenterDis / FrontTargetTimes + CosCenterAngle;
	float FocusInterval = FrontTargetSum / (FrontTargetSum + BackTargetSum);
	float CenterInterval = Dis * FocusInterval * BackTargetSum;
	float BackWidthToEdge = (CenterInterval + Dis * FocusInterval * CosCenterAngle) * HalfWidthDivideCenterDis;
	float FontWidthToEdge = (CenterInterval - Dis * (1.f - FocusInterval) * CosCenterAngle) * HalfWidthDivideCenterDis;
	float BackTargetProjector = Dis * FocusInterval * SinCenterAngle;
	float FrontTargetProjector = Dis * (1 - FocusInterval) * SinCenterAngle;

	FVector LookCenter = BackTargetPosition * (1 - FocusInterval) + FrontTargetPosition * FocusInterval;
	CameraPosition = LookCenter + CenterInterval * FRotator(0.f, CameraYawAngle, 0.f).RotateVector((FrontTargetPosition - BackTargetPosition)).GetSafeNormal();
	CameraRotation = (LookCenter - CameraPosition).Rotation();
}
