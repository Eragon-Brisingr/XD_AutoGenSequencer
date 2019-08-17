﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovieSceneTrackEditor.h"

/**
 * 
 */
class XD_AUTOGENSEQUENCER_EDITOR_API FDialogueSentenceTrackEditor : public FMovieSceneTrackEditor
{
public:
	FDialogueSentenceTrackEditor(TSharedRef<ISequencer> InSequencer)
		:FMovieSceneTrackEditor(InSequencer)
	{}

	void AddKey(const FGuid& ObjectGuid) override;
	void BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const TArray<FGuid>& ObjectBindings, const UClass* ObjectClass) override;
	bool HandleAssetAdded(UObject* Asset, const FGuid& TargetObjectGuid) override;
	TSharedRef<ISequencerSection> MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding) override;
	bool SupportsType(TSubclassOf<UMovieSceneTrack> Type) const override;
	void BuildTrackContextMenu(FMenuBuilder& MenuBuilder, UMovieSceneTrack* Track) override;
	TSharedPtr<SWidget> BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params) override;
	bool OnAllowDrop(const FDragDropEvent& DragDropEvent, UMovieSceneTrack* Track, int32 RowIndex, const FGuid& TargetObjectGuid) override;
	FReply OnDrop(const FDragDropEvent& DragDropEvent, UMovieSceneTrack* Track, int32 RowIndex, const FGuid& TargetObjectGuid) override;
};

class FDialogueSentenceSection
	: public ISequencerSection
	, public TSharedFromThis<FDialogueSentenceSection>
{
public:

	/** Constructor. */
	FDialogueSentenceSection(UMovieSceneSection& InSection, TWeakPtr<ISequencer> InSequencer);

	/** Virtual destructor. */
	virtual ~FDialogueSentenceSection() { }

// public:
// 
// 	// ISequencerSection interface
// 
 	UMovieSceneSection* GetSectionObject() override;
// 	FText GetSectionTitle() const override;
// 	float GetSectionHeight() const override;
// 	FMargin GetContentPadding() const override;
 	int32 OnPaintSection(FSequencerSectionPainter& Painter) const override;
// 	void BeginResizeSection() override;
// 	void ResizeSection(ESequencerSectionResizeMode ResizeMode, FFrameNumber ResizeTime) override;
// 	void BeginSlipSection() override;
// 	void SlipSection(FFrameNumber SlipTime) override;
// 	void BuildSectionContextMenu(FMenuBuilder& MenuBuilder, const FGuid& InObjectBinding) override;
// 
// private:
// 
// 	bool CreatePoseAsset(const TArray<UObject*> NewAssets, FGuid InObjectBinding);
// 	void HandleCreatePoseAsset(FGuid InObjectBinding);
// 
 private:
 
 	/** The section we are visualizing */
	UMovieSceneSection& Section;
 
 	/** Used to draw animation frame, need selection state and local time*/
 	TWeakPtr<ISequencer> Sequencer;
 
 	/** Cached start offset value valid only during resize */
 	FFrameNumber InitialStartOffsetDuringResize;
 
 	/** Cached start time valid only during resize */
 	FFrameNumber InitialStartTimeDuringResize;
};
