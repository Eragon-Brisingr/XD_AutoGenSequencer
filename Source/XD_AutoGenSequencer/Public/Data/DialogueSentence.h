// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <UObject/NoExportTypes.h>
#include "DialogueSentence.generated.h"

class USoundWave;
class UDialogueSentenceSection;

/**
 * 
 */
UCLASS(BlueprintType)
class XD_AUTOGENSEQUENCER_API UDialogueSentence : public UObject
{
	GENERATED_BODY()
public:
	UDialogueSentence();

	UPROPERTY(EditAnywhere, Category = "对话", meta = (DisplayName = "音频"))
	USoundWave* SentenceWave;
	UPROPERTY(EditAnywhere, Category = "对话", meta = (DisplayName = "字幕"))
	FText SubTitle;

	UFUNCTION(BlueprintCallable, meta = (CompactNodeTitle = "SubTitle"), Category = "对话")
	FORCEINLINE FText GetSubTitle() const { return SubTitle; }

	float GetDuration() const;

#if WITH_EDITOR
	virtual void WhenGenFromSoundWave(USoundWave* InSentenceWave) {}
#endif

	virtual TSubclassOf<UDialogueSentenceSection> GetSectionImplType() const;

public:
#if WITH_EDITOR
	static FName AssetRegistryTag_SubTitle;
	void GetAssetRegistryTagMetadata(TMap<FName, FAssetRegistryTagMetadata>& OutMetadata) const override;
	void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
#endif
};
