// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueSentence.h"
#include "DialogueSentenceSection.h"

UDialogueSentence::UDialogueSentence()
{
	
}

TSubclassOf<UDialogueSentenceSection> UDialogueSentence::GetSectionImplType() const
{
	return UDialogueSentenceSection::StaticClass();
}
