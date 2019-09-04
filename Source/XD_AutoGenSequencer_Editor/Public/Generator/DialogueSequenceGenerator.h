// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UPreviewDialogueSoundSequence;
class UAutoGenDialogueSequence;
class ACharacter;

/**
 * 
 */
class XD_AUTOGENSEQUENCER_EDITOR_API FDialogueSequenceGenerator
{
public:
	static FDialogueSequenceGenerator& Get()
	{
		static FDialogueSequenceGenerator PreviewDialogueGenerator;
		return PreviewDialogueGenerator;
	}

	void Generate(TSharedRef<ISequencer> SequencerRef, UWorld* World, const TMap<FName, TSoftObjectPtr<ACharacter>>& CharacterNameInstanceMap, 
		const UAutoGenDialogueSequenceConfig& GenConfig, const UPreviewDialogueSoundSequence* PreviewDialogueSoundSequence, UAutoGenDialogueSequence* AutoGenDialogueSequence);
};
