// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelSequence.h"
#include "PreviewDialogueSoundSequence.generated.h"

class UPreviewDialogueSentenceTrack;
class UAutoGenDialogueSystemData;
class UAutoGenDialogueSequenceConfig;

/**
 * 
 */
UCLASS()
class XD_AUTOGENSEQUENCER_EDITOR_API UPreviewDialogueSoundSequence : public ULevelSequence
{
	GENERATED_BODY()
public:
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TArray<UPreviewDialogueSentenceTrack*> PreviewDialogueSentenceTracks;

	UGenDialogueSequenceConfigBase* GetDialogueConfig() const;

	bool HasPreviewData() const;
#endif
};
