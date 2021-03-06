﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Templates/SubclassOf.h>
#include "AutoGenDialogueType.generated.h"

class UDialogueSentence;
class USoundBase;

/**
 *
 */
USTRUCT()
struct AUTOGENSEQUENCER_EDITOR_API FDialogueCharacterName
{
	GENERATED_BODY()
public:
	FDialogueCharacterName(const FName& Name = NAME_None)
		:Name(Name)
	{}

	UPROPERTY(EditAnywhere)
	FName Name;

	friend bool operator==(const FDialogueCharacterName& LHS, const FDialogueCharacterName& RHS) { return LHS.Name == RHS.Name; }
	friend uint32 GetTypeHash(const FDialogueCharacterName& DialogueCharacterName) { return GetTypeHash(DialogueCharacterName.Name); }
	FORCEINLINE FName GetName() const { return Name; }
};

USTRUCT()
struct AUTOGENSEQUENCER_EDITOR_API FDialogueSentenceEditData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "对白"))
	UDialogueSentence* DialogueSentence;
	UPROPERTY(EditAnywhere, meta = (DisplayName = "说话者"))
	FDialogueCharacterName SpeakerName = TEXT("Role");
	UPROPERTY(EditAnywhere, meta = (DisplayName = "向所有人说"))
	uint8 bToAllTargets : 1;
	UPROPERTY(EditAnywhere, meta = (DisplayName = "对白目标", EditCondition = "!bToAllTargets"))
	TArray<FDialogueCharacterName> TargetNames = { FDialogueCharacterName(TEXT("Target1")) };
public:
	USoundBase* GetDefaultDialogueSound() const;
};
