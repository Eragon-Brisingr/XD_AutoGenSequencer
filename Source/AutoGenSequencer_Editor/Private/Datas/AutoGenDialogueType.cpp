﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "Datas/AutoGenDialogueType.h"
#include "Data/DialogueSentence.h"

USoundBase* FDialogueSentenceEditData::GetDefaultDialogueSound() const
{
	return DialogueSentence->SentenceWave;
}
