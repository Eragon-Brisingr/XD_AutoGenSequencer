// Fill out your copyright notice in the Description page of Project Settings.


#include "PreviewDialogueSoundSequence.h"

#if WITH_EDITOR
UGenDialogueSequenceConfigBase* UPreviewDialogueSoundSequence::GetDialogueConfig() const
{
	return GetTypedOuter<UGenDialogueSequenceConfigBase>();
}
#endif