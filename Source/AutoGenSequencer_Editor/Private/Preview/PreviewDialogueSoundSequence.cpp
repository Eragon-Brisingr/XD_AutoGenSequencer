// Fill out your copyright notice in the Description page of Project Settings.


#include "Preview/PreviewDialogueSoundSequence.h"
#include "MovieScene.h"
#include "AutoGenEditor/GenDialogueSequenceConfigBase.h"

UGenDialogueSequenceConfigBase* UPreviewDialogueSoundSequence::GetDialogueConfig() const
{
	return GetTypedOuter<UGenDialogueSequenceConfigBase>();
}

bool UPreviewDialogueSoundSequence::HasPreviewData() const
{
	return CharacterGuids.Num() > 0;
}
