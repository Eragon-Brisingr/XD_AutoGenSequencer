// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovieSceneNameableTrack.h"
#include "CameraTrackingTrackBase.generated.h"

/**
 * 
 */
UCLASS(abstract)
class AUTOGENSEQUENCER_API UCameraTrackingTrackBase : public UMovieSceneNameableTrack
{
	GENERATED_BODY()
public:
	void RemoveAllAnimationData() override;
	bool HasSection(const UMovieSceneSection& Section) const override;
	void AddSection(UMovieSceneSection& Section) override;
	void RemoveSection(UMovieSceneSection& Section) override;
	void RemoveSectionAt(int32 SectionIndex) override;
	bool IsEmpty() const override;
	const TArray<UMovieSceneSection*>& GetAllSections() const override;
	bool SupportsMultipleRows() const override;
	FMovieSceneTrackRowSegmentBlenderPtr GetRowSegmentBlender() const override;
protected:
	UPROPERTY()
	TArray<UMovieSceneSection*> CameraTrackingSections;
};
