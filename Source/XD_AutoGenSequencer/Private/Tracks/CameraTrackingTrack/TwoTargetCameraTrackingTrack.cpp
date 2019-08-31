// Fill out your copyright notice in the Description page of Project Settings.


#include "TwoTargetCameraTrackingTrack.h"
#include "TwoTargetCameraTrackingSection.h"

#define LOCTEXT_NAMESPACE "TwoTargetCameraTrackingTrack"

UTwoTargetCameraTrackingTrack::UTwoTargetCameraTrackingTrack(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
#if WITH_EDITORONLY_DATA
	SetDisplayName(LOCTEXT("TwoTargetCameraTrackingTrackName", "双目标追踪"));
#endif
}

bool UTwoTargetCameraTrackingTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == UTwoTargetCameraTrackingSection::StaticClass();
}

void UTwoTargetCameraTrackingTrack::RemoveAllAnimationData()
{
	CameraTrackingSections.Empty();
}

bool UTwoTargetCameraTrackingTrack::HasSection(const UMovieSceneSection& Section) const
{
	return CameraTrackingSections.Contains(&Section);
}

void UTwoTargetCameraTrackingTrack::AddSection(UMovieSceneSection& Section)
{
	CameraTrackingSections.Add(&Section);
}

void UTwoTargetCameraTrackingTrack::RemoveSection(UMovieSceneSection& Section)
{
	CameraTrackingSections.Remove(&Section);
}

void UTwoTargetCameraTrackingTrack::RemoveSectionAt(int32 SectionIndex)
{
	CameraTrackingSections.RemoveAt(SectionIndex);
}

bool UTwoTargetCameraTrackingTrack::IsEmpty() const
{
	return CameraTrackingSections.Num() == 0;
}

const TArray<UMovieSceneSection*>& UTwoTargetCameraTrackingTrack::GetAllSections() const
{
	return CameraTrackingSections;
}

bool UTwoTargetCameraTrackingTrack::SupportsMultipleRows() const
{
	return false;
}

FMovieSceneTrackRowSegmentBlenderPtr UTwoTargetCameraTrackingTrack::GetRowSegmentBlender() const
{
	return Super::GetRowSegmentBlender();
}

UMovieSceneSection* UTwoTargetCameraTrackingTrack::CreateNewSection()
{
	UTwoTargetCameraTrackingSection* NewSection = NewObject<UTwoTargetCameraTrackingSection>(this, NAME_None, RF_Transactional);
	NewSection->SetRange(TRange<FFrameNumber>::All());
	return NewSection;
}

UTwoTargetCameraTrackingSection* UTwoTargetCameraTrackingTrack::AddNewSentenceOnRow(FMovieSceneObjectBindingID FrontTarget, FMovieSceneObjectBindingID BackTarget)
{
	UTwoTargetCameraTrackingSection* NewSection = NewObject<UTwoTargetCameraTrackingSection>(this, NAME_None, RF_Transactional);
	NewSection->SetRange(TRange<FFrameNumber>::All());
	AddSection(*NewSection);
	NewSection->FrontTarget = FrontTarget;
	NewSection->BackTarget = BackTarget;
	return NewSection;
}

#undef LOCTEXT_NAMESPACE
