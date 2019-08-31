// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DialogueInterface.generated.h"

class UAudioComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDialogueInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class XD_AUTOGENSEQUENCER_API IDialogueInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Dialogue", meta = (DispalyName = GetMouthComponent))
	UAudioComponent* GetMouthComponent() const;
	virtual UAudioComponent* GetMouthComponent_Implementation() const;
	static UAudioComponent* GetMouthComponent(const UObject* Obj);
	UFUNCTION(BlueprintNativeEvent, Category = "Dialogue", meta = (DispalyName = GetDialogueCharacterName))
	FName GetDialogueCharacterName() const;
	virtual FName GetDialogueCharacterName_Implementation() const;
	static FName GetDialogueCharacterName(const UObject* Obj);
};
