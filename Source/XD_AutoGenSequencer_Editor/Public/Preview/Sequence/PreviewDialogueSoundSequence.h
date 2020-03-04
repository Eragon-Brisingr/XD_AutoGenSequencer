// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <LevelSequence.h>
#include "PreviewDialogueSoundSequence.generated.h"

class UGenDialogueSequenceConfigBase;
class UDialogueSentenceTrack;

/**
 * 
 */
UCLASS()
class XD_AUTOGENSEQUENCER_EDITOR_API UPreviewDialogueSoundSequence : public ULevelSequence
{
	GENERATED_BODY()
public:
	UGenDialogueSequenceConfigBase* GetDialogueConfig() const;

	bool HasPreviewData() const;

public:
	UPROPERTY()
	TMap<FName, UDialogueSentenceTrack*> DialogueSentenceTracks;

	UPROPERTY()
	TArray<FGuid> CharacterGuids;
};
