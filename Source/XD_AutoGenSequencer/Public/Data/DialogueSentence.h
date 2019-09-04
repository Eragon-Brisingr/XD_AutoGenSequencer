// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DialogueSentence.generated.h"

class USoundWave;

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
	USoundWave* SentenceWave;
	UPROPERTY(EditAnywhere, Category = "DialogueSentence")
	FText SubTitle;

	FORCEINLINE FText GetSubTitle() const { return SubTitle; }

#if WITH_EDITOR
	virtual void WhenGenFromSoundWave(USoundWave* InSentenceWave) {}
#endif
};
