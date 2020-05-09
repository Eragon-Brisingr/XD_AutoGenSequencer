// Fill out your copyright notice in the Description page of Project Settings.


#include "Tracks/CameraTrackingTrack/TwoTargetCameraTrackingTrack.h"
#include "Tracks/CameraTrackingTrack/TwoTargetCameraTrackingSection.h"

#define LOCTEXT_NAMESPACE "TwoTargetCameraTrackingTrack"

UTwoTargetCameraTrackingTrack::UTwoTargetCameraTrackingTrack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	SetDisplayName(LOCTEXT("双目标追踪", "双目标追踪"));
#endif
}

bool UTwoTargetCameraTrackingTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == UTwoTargetCameraTrackingSection::StaticClass();
}

UMovieSceneSection* UTwoTargetCameraTrackingTrack::CreateNewSection()
{
	UTwoTargetCameraTrackingSection* NewSection = NewObject<UTwoTargetCameraTrackingSection>(this, NAME_None, RF_Transactional);
	NewSection->SetRange(TRange<FFrameNumber>::All());
	return NewSection;
}

UTwoTargetCameraTrackingSection* UTwoTargetCameraTrackingTrack::AddNewSentenceOnRow(FMovieSceneObjectBindingID FrontTarget, const FVector& FrontOffset, FMovieSceneObjectBindingID BackTarget, const FVector& BackOffset)
{
	UTwoTargetCameraTrackingSection* NewSection = NewObject<UTwoTargetCameraTrackingSection>(this, NAME_None, RF_Transactional);
	NewSection->SetRange(TRange<FFrameNumber>::All());
	AddSection(*NewSection);
	NewSection->FrontTarget = FrontTarget;
	NewSection->FrontOffset = FrontOffset;
	NewSection->BackTarget = BackTarget;
	NewSection->BackOffset = BackOffset;
	return NewSection;
}

#undef LOCTEXT_NAMESPACE
