// Fill out your copyright notice in the Description page of Project Settings.


#include "PreviewDialogueSoundSequence.h"
#include "AutoGenDialogueSequence.h"

#if WITH_EDITOR
UAutoGenDialogueSequence* UPreviewDialogueSoundSequence::GetAutoGenDialogueSequence() const
{
	return GetTypedOuter<UAutoGenDialogueSequence>();
}

UAutoGenDialogueSequenceConfig* UPreviewDialogueSoundSequence::GetDialogueConfig() const
{
	return GetAutoGenDialogueSequence()->AutoGenDialogueSequenceConfig;
}
#endif