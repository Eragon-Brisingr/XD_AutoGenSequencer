// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AutoGenDialogueInterface.generated.h"

class UAudioComponent;
class UDialogueSentence;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAutoGenDialogueInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AUTOGENSEQUENCER_API IAutoGenDialogueInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Dialogue")
	UAudioComponent* GetDialogueMouthComponent() const;
	virtual UAudioComponent* GetDialogueMouthComponent_Implementation() const;
	static UAudioComponent* GetDialogueMouthComponent(const UObject* Obj);
	UFUNCTION(BlueprintNativeEvent, Category = "Dialogue")
	FName GetDialogueCharacterName() const;
	virtual FName GetDialogueCharacterName_Implementation() const;
	static FName GetDialogueCharacterName(const UObject* Obj);

	UFUNCTION(BlueprintNativeEvent, Category = "Dialogue")
	void BeginDialogueSpeak(UDialogueSentence* Sentence);
	virtual void BeginDialogueSpeak_Implementation(UDialogueSentence* Sentence) {}
	static void BeginDialogueSpeak(UObject* Obj, UDialogueSentence* Sentence);

	UFUNCTION(BlueprintNativeEvent, Category = "Dialogue")
	void EndDialogueSpeak();
	virtual void EndDialogueSpeak_Implementation() {}
	static void EndDialogueSpeak(UObject* Obj);
};
