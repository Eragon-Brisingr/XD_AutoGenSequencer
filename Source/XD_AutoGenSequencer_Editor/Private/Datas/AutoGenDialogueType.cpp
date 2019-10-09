// Fill out your copyright notice in the Description page of Project Settings.

#include "AutoGenDialogueType.h"
#include "DialogueSentence.h"

USoundBase* FDialogueSentenceEditData::GetDefaultDialogueSound() const
{
	return DialogueSentence->SentenceWave;
}
