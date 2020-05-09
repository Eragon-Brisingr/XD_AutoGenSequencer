// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tracks/CameraTrackingTrack/CameraTrackingTrackBase.h"
#include "OneTargetCameraTrackingTrack.generated.h"

class UOneTargetCameraTrackingSection;

/**
 * 
 */
UCLASS(meta = (DisplayName = "单目标追踪导轨"))
class AUTOGENSEQUENCER_API UOneTargetCameraTrackingTrack : public UCameraTrackingTrackBase
{
	GENERATED_BODY()
public:
	UOneTargetCameraTrackingTrack(const FObjectInitializer& ObjectInitializer);

	bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override;
	UMovieSceneSection* CreateNewSection() override;
public:
	UOneTargetCameraTrackingSection* AddNewSentenceOnRow(FMovieSceneObjectBindingID Target);
};
