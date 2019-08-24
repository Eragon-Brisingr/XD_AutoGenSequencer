// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class UAutoGenDialogueSequenceConfig;
class UPreviewDialogueSoundSequence;

/**
 * 
 */
class XD_AUTOGENSEQUENCER_EDITOR_API FPreviewDialogueGenerator
{
public:
	static FPreviewDialogueGenerator& Get()
	{
		static FPreviewDialogueGenerator PreviewDialogueGenerator;
		return PreviewDialogueGenerator;
	}

	void Generate(const UAutoGenDialogueSequenceConfig* Config, UPreviewDialogueSoundSequence* PreviewDialogueSoundSequence);

	//对话间隔时间
	float PaddingTime = 0.6f;
};
