// Fill out your copyright notice in the Description page of Project Settings.


#include "Preview/Sequence/PreviewDialogueSoundSequence.h"
#include "MovieScene.h"
#include "Datas/GenDialogueSequenceConfigBase.h"

#if WITH_EDITOR
UGenDialogueSequenceConfigBase* UPreviewDialogueSoundSequence::GetDialogueConfig() const
{
	return GetTypedOuter<UGenDialogueSequenceConfigBase>();
}

bool UPreviewDialogueSoundSequence::HasPreviewData() const
{
	return MovieScene->GetMasterTracks().Num() > 0;
}

#endif