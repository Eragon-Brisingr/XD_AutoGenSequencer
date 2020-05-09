// Fill out your copyright notice in the Description page of Project Settings.


#include "Tracks/CameraTrackingTrack/OneTargetCameraTrackingTrack.h"
#include "Tracks/CameraTrackingTrack/OneTargetCameraTrackingSection.h"

#define LOCTEXT_NAMESPACE "OneTargetCameraTrackingTrack"

UOneTargetCameraTrackingTrack::UOneTargetCameraTrackingTrack(const FObjectInitializer& ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	SetDisplayName(LOCTEXT("单目标追踪", "单目标追踪"));
#endif
}

bool UOneTargetCameraTrackingTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == UOneTargetCameraTrackingSection::StaticClass();
}

UMovieSceneSection* UOneTargetCameraTrackingTrack::CreateNewSection()
{
	UOneTargetCameraTrackingSection* NewSection = NewObject<UOneTargetCameraTrackingSection>(this, NAME_None, RF_Transactional);
	NewSection->SetRange(TRange<FFrameNumber>::All());
	return NewSection;
}

UOneTargetCameraTrackingSection* UOneTargetCameraTrackingTrack::AddNewSentenceOnRow(FMovieSceneObjectBindingID Target)
{
	UOneTargetCameraTrackingSection* NewSection = NewObject<UOneTargetCameraTrackingSection>(this, NAME_None, RF_Transactional);
	NewSection->SetRange(TRange<FFrameNumber>::All());
	NewSection->Target = Target;
	AddSection(*NewSection);
	return NewSection;
}

#undef LOCTEXT_NAMESPACE
