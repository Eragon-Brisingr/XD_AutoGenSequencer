// Fill out your copyright notice in the Description page of Project Settings.

#include "Tracks/CameraTrackingTrack/CameraTrackingTrackBase.h"

void UCameraTrackingTrackBase::RemoveAllAnimationData()
{
	CameraTrackingSections.Empty();
}

bool UCameraTrackingTrackBase::HasSection(const UMovieSceneSection& Section) const
{
	return CameraTrackingSections.Contains(&Section);
}

void UCameraTrackingTrackBase::AddSection(UMovieSceneSection& Section)
{
	CameraTrackingSections.Add(&Section);
}

void UCameraTrackingTrackBase::RemoveSection(UMovieSceneSection& Section)
{
	CameraTrackingSections.Remove(&Section);
}

void UCameraTrackingTrackBase::RemoveSectionAt(int32 SectionIndex)
{
	CameraTrackingSections.RemoveAt(SectionIndex);
}

bool UCameraTrackingTrackBase::IsEmpty() const
{
	return CameraTrackingSections.Num() == 0;
}

const TArray<UMovieSceneSection*>& UCameraTrackingTrackBase::GetAllSections() const
{
	return CameraTrackingSections;
}

bool UCameraTrackingTrackBase::SupportsMultipleRows() const
{
	return false;
}

FMovieSceneTrackRowSegmentBlenderPtr UCameraTrackingTrackBase::GetRowSegmentBlender() const
{
	return Super::GetRowSegmentBlender();
}
