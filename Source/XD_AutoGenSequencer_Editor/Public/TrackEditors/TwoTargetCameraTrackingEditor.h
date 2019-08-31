// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovieSceneTrackEditor.h"

class UTwoTargetCameraTrackingSection;

/**
 * 
 */
class XD_AUTOGENSEQUENCER_EDITOR_API FTwoTargetCameraTrackingEditor : public FMovieSceneTrackEditor
{
public:
	FTwoTargetCameraTrackingEditor(TSharedRef<ISequencer> InSequencer)
		:FMovieSceneTrackEditor(InSequencer)
	{}

	void BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const TArray<FGuid>& ObjectBindings, const UClass* ObjectClass) override;
	TSharedPtr<SWidget> BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params) override;
	bool SupportsType(TSubclassOf<UMovieSceneTrack> Type) const override;
	bool SupportsSequence(UMovieSceneSequence* InSequence) const override;
};

class XD_AUTOGENSEQUENCER_EDITOR_API FTwoTargetCameraTrackingSectionEditor
	: public ISequencerSection
	, public TSharedFromThis<FTwoTargetCameraTrackingSectionEditor>
{
public:
	/** Constructor. */
	FTwoTargetCameraTrackingSectionEditor(UMovieSceneSection& InSection, TWeakPtr<ISequencer> InSequencer);

public:
	// ISequencerSection interface
	UMovieSceneSection* GetSectionObject() override;
	FText GetSectionTitle() const override;
	float GetSectionHeight() const override;
	int32 OnPaintSection(FSequencerSectionPainter& Painter) const override;
	void BeginResizeSection() override;
	void ResizeSection(ESequencerSectionResizeMode ResizeMode, FFrameNumber ResizeTime) override;
	void BeginSlipSection() override;
	void SlipSection(FFrameNumber SlipTime) override;

	/** The section we are visualizing */
	UTwoTargetCameraTrackingSection& Section;

	/** Used to draw animation frame, need selection state and local time*/
	TWeakPtr<ISequencer> Sequencer;

	/** Cached start offset value valid only during resize */
	FFrameNumber InitialStartOffsetDuringResize;

	/** Cached start time valid only during resize */
	FFrameNumber InitialStartTimeDuringResize;
};
