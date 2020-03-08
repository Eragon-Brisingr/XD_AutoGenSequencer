// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tracks/CameraTrackingTrack/CameraTrackingTrackBase.h"
#include "TwoTargetCameraTrackingTrack.generated.h"

class UTwoTargetCameraTrackingSection;

/**
 * 
 */
UCLASS(meta = (DisplayName = "双目标追踪导轨"))
class XD_AUTOGENSEQUENCER_API UTwoTargetCameraTrackingTrack : public UCameraTrackingTrackBase
{
	GENERATED_BODY()
public:
	UTwoTargetCameraTrackingTrack(const FObjectInitializer& ObjectInitializer);

	bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override;
	UMovieSceneSection* CreateNewSection() override;
public:
	UTwoTargetCameraTrackingSection* AddNewSentenceOnRow(FMovieSceneObjectBindingID FrontTarget, const FVector& FrontOffset, FMovieSceneObjectBindingID BackTarget, const FVector& BackOffset);
};
