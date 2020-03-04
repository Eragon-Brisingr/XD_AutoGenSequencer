// Fill out your copyright notice in the Description page of Project Settings.


#include "Interface/XD_AutoGenDialogueInterface.h"
#include <GameFramework/Actor.h>
#include <Components/AudioComponent.h>
#include <UObject/Script.h>

// Add default functionality here for any IDialogueInterface functions that are not pure virtual.

UAudioComponent* IXD_AutoGenDialogueInterface::GetDialogueMouthComponent_Implementation() const
{
	return Cast<AActor>(this)->FindComponentByClass<UAudioComponent>();
}

UAudioComponent* IXD_AutoGenDialogueInterface::GetDialogueMouthComponent(const UObject* Obj)
{
	FEditorScriptExecutionGuard ScriptGuard;
	return IXD_AutoGenDialogueInterface::Execute_GetDialogueMouthComponent(const_cast<UObject*>(Obj));
}

FName IXD_AutoGenDialogueInterface::GetDialogueCharacterName_Implementation() const
{
	return Cast<UObject>(this)->GetFName();
}

FName IXD_AutoGenDialogueInterface::GetDialogueCharacterName(const UObject* Obj)
{
	FEditorScriptExecutionGuard ScriptGuard;
	return IXD_AutoGenDialogueInterface::Execute_GetDialogueCharacterName(const_cast<UObject*>(Obj));
}

void IXD_AutoGenDialogueInterface::BeginDialogueSpeak(UObject* Obj, UDialogueSentence* Sentence)
{
	FEditorScriptExecutionGuard ScriptGuard;
	return IXD_AutoGenDialogueInterface::Execute_BeginDialogueSpeak(Obj, Sentence);
}

void IXD_AutoGenDialogueInterface::EndDialogueSpeak(UObject* Obj)
{
	FEditorScriptExecutionGuard ScriptGuard;
	return IXD_AutoGenDialogueInterface::Execute_EndDialogueSpeak(Obj);
}
