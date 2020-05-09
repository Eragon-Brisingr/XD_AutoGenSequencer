// Fill out your copyright notice in the Description page of Project Settings.


#include "Interface/AutoGenDialogueInterface.h"
#include <GameFramework/Actor.h>
#include <Components/AudioComponent.h>
#include <UObject/Script.h>

// Add default functionality here for any IDialogueInterface functions that are not pure virtual.

UAudioComponent* IAutoGenDialogueInterface::GetDialogueMouthComponent_Implementation() const
{
	return Cast<AActor>(this)->FindComponentByClass<UAudioComponent>();
}

UAudioComponent* IAutoGenDialogueInterface::GetDialogueMouthComponent(const UObject* Obj)
{
	FEditorScriptExecutionGuard ScriptGuard;
	return IAutoGenDialogueInterface::Execute_GetDialogueMouthComponent(const_cast<UObject*>(Obj));
}

FName IAutoGenDialogueInterface::GetDialogueCharacterName_Implementation() const
{
	return Cast<UObject>(this)->GetFName();
}

FName IAutoGenDialogueInterface::GetDialogueCharacterName(const UObject* Obj)
{
	FEditorScriptExecutionGuard ScriptGuard;
	return IAutoGenDialogueInterface::Execute_GetDialogueCharacterName(const_cast<UObject*>(Obj));
}

void IAutoGenDialogueInterface::BeginDialogueSpeak(UObject* Obj, UDialogueSentence* Sentence)
{
	FEditorScriptExecutionGuard ScriptGuard;
	return IAutoGenDialogueInterface::Execute_BeginDialogueSpeak(Obj, Sentence);
}

void IAutoGenDialogueInterface::EndDialogueSpeak(UObject* Obj)
{
	FEditorScriptExecutionGuard ScriptGuard;
	return IAutoGenDialogueInterface::Execute_EndDialogueSpeak(Obj);
}
