// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueInterface.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Script.h"

// Add default functionality here for any IDialogueInterface functions that are not pure virtual.

UAudioComponent* IDialogueInterface::GetMouthComponent_Implementation() const
{
	return Cast<AActor>(this)->FindComponentByClass<UAudioComponent>();
}

UAudioComponent* IDialogueInterface::GetMouthComponent(const UObject* Obj)
{
	FEditorScriptExecutionGuard ScriptGuard;
	return IDialogueInterface::Execute_GetMouthComponent(const_cast<UObject*>(Obj));
}

FName IDialogueInterface::GetDialogueCharacterName_Implementation() const
{
	return Cast<UObject>(this)->GetFName();
}

FName IDialogueInterface::GetDialogueCharacterName(const UObject* Obj)
{
	FEditorScriptExecutionGuard ScriptGuard;
	return IDialogueInterface::Execute_GetDialogueCharacterName(const_cast<UObject*>(Obj));
}

void IDialogueInterface::BeginSpeak(UObject* Obj, UDialogueSentence* Sentence)
{
	return IDialogueInterface::Execute_BeginSpeak(Obj, Sentence);
}

void IDialogueInterface::EndSpeak(UObject* Obj)
{
	return IDialogueInterface::Execute_EndSpeak(Obj);
}
