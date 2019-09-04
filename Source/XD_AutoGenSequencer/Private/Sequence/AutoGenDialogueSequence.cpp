// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoGenDialogueSequence.h"
#include "PreviewDialogueSoundSequence.h"
#include "AutoGenDialogueSequenceConfig.h"
#include "PreviewDialogueSentenceTrack.h"

UAutoGenDialogueSequence::UAutoGenDialogueSequence()
{

}

#if WITH_EDITOR
FGuid UAutoGenDialogueSequence::FindOrAddPossessable(UObject* ObjectToPossess)
{
	return CreatePossessable(ObjectToPossess);
}
#endif
