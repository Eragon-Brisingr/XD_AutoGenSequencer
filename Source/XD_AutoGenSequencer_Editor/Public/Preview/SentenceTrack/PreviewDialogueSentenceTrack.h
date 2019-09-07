// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovieSceneAudioTrack.h"
#include "PreviewDialogueSentenceTrack.generated.h"

struct FDialogueSentenceEditData;

/**
 * 
 */
UCLASS()
class XD_AUTOGENSEQUENCER_EDITOR_API UPreviewDialogueSentenceTrack : public UMovieSceneAudioTrack
{
	GENERATED_BODY()
public:
	UPreviewDialogueSentenceTrack();

	bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override;
	UMovieSceneSection* CreateNewSection() override;
	UMovieSceneSection* AddNewDialogueOnRow(const FDialogueSentenceEditData& DialogueSentenceEditData, FFrameNumber Time, FFrameNumber& DurationTime, int32 RowIndex = INDEX_NONE);
	bool SupportsMultipleRows() const override;

public:
	UPROPERTY()
	FName SpeakerName;
};
