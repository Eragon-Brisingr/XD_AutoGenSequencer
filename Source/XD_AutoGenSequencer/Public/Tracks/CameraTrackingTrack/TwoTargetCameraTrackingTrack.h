// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovieSceneNameableTrack.h"
#include "TwoTargetCameraTrackingTrack.generated.h"

class UTwoTargetCameraTrackingSection;

/**
 * 
 */
UCLASS()
class XD_AUTOGENSEQUENCER_API UTwoTargetCameraTrackingTrack : public UMovieSceneNameableTrack
{
	GENERATED_BODY()
public:
	UTwoTargetCameraTrackingTrack(const FObjectInitializer& ObjectInitializer);

	bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override;
	void RemoveAllAnimationData() override;
	bool HasSection(const UMovieSceneSection& Section) const override;
	void AddSection(UMovieSceneSection& Section) override;
	void RemoveSection(UMovieSceneSection& Section) override;
	void RemoveSectionAt(int32 SectionIndex) override;
	bool IsEmpty() const override;
	const TArray<UMovieSceneSection*>& GetAllSections() const override;
	bool SupportsMultipleRows() const override;
	FMovieSceneTrackRowSegmentBlenderPtr GetRowSegmentBlender() const override;
	UMovieSceneSection* CreateNewSection() override;
public:
	UPROPERTY()
	TArray<UMovieSceneSection*> CameraTrackingSections;
public:
	UTwoTargetCameraTrackingSection* AddNewSentenceOnRow(FMovieSceneObjectBindingID FrontTarget, const FVector& FrontOffset, FMovieSceneObjectBindingID BackTarget, const FVector& BackOffset);
};
