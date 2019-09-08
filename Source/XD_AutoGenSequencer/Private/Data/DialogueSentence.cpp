// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueSentence.h"
#include "DialogueSentenceSection.h"
#include "Sound/SoundWave.h"

UDialogueSentence::UDialogueSentence()
{
	
}

float UDialogueSentence::GetDuration() const
{
	return SentenceWave->GetDuration();
}

TSubclassOf<UDialogueSentenceSection> UDialogueSentence::GetSectionImplType() const
{
	return UDialogueSentenceSection::StaticClass();
}
