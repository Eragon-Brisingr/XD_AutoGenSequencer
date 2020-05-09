// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/DialogueSentence.h"
#include <Sound/SoundWave.h>
#include "Tracks/SentenceTrack/DialogueSentenceSection.h"

UDialogueSentence::UDialogueSentence()
{
	
}

float UDialogueSentence::GetDuration() const
{
	return SentenceWave->GetDuration();
}

TSubclassOf<UDialogueSentenceSection> UDialogueSentence::GetSectionImplType() const
{
	return UDialogueSentenceSection::StaticClass();
}

#if WITH_EDITOR

FName UDialogueSentence::AssetRegistryTag_SubTitle = TEXT("DialogueSentence_SubTitle");

void UDialogueSentence::GetAssetRegistryTagMetadata(TMap<FName, FAssetRegistryTagMetadata>& OutMetadata) const
{
	Super::GetAssetRegistryTagMetadata(OutMetadata);

	if (!SubTitle.IsEmpty())
	{
		OutMetadata.Add(AssetRegistryTag_SubTitle, FAssetRegistryTagMetadata()
			.SetDisplayName(NSLOCTEXT("DialogueSentenceMetaData", "SubTitle_Label", "SubTitle"))
			.SetTooltip(NSLOCTEXT("DialogueSentenceMetaData", "SubTitle_Tip", "SubTitle")));
	}
}

void UDialogueSentence::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	Super::GetAssetRegistryTags(OutTags);

	if (!SubTitle.IsEmpty())
	{
		OutTags.Emplace(AssetRegistryTag_SubTitle, SubTitle.ToString(), FAssetRegistryTag::ETagType::TT_Numerical, FAssetRegistryTag::TD_None);
	}
}

#endif // WITH_EDITOR
