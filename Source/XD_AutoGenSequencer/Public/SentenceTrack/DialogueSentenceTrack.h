// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovieSceneNameableTrack.h"
#include "DialogueSentenceTrack.generated.h"

class UDialogueSentenceSection;

/**
 * 
 */
UCLASS()
class XD_AUTOGENSEQUENCER_API UDialogueSentenceTrack : public UMovieSceneNameableTrack
{
	GENERATED_BODY()
public:
	UDialogueSentenceTrack(const FObjectInitializer& ObjectInitializer);

	// UMovieSceneTrack interface

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
	TArray<UMovieSceneSection*> SentenceSections;

#if WITH_EDITORONLY_DATA
public:
	int32 GetRowHeight() const { return RowHeight; }

	void SetRowHeight(int32 NewRowHeight) { RowHeight = FMath::Max(16, NewRowHeight); }
private:
	UPROPERTY()
	int32 RowHeight = 16;
#endif
};
