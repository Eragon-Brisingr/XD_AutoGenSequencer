// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DialogueSentence.generated.h"

class USoundBase;

/**
 * 
 */
UCLASS()
class XD_AUTOGENSEQUENCER_API UDialogueSentence : public UObject
{
	GENERATED_BODY()
public:
	UDialogueSentence();

	UPROPERTY(EditAnywhere, Category = "DialogueSentence")
	USoundBase* SentenceWave;
	UPROPERTY(EditAnywhere, Category = "DialogueSentence")
	FText SubTitle;

	FORCEINLINE FText GetSubTitle() const { return SubTitle; }
};
