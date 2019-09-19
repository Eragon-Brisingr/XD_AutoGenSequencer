// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ILevelSequenceMetaData.h"
#include "AutoGenDialogueSystemData.generated.h"

class UMovieSceneTrack;
class ULevelSequence;

/**
 * 
 */
UCLASS()
class XD_AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueSystemData : public UObject, public ILevelSequenceMetaData
{
	GENERATED_BODY()
public:
	// TODO：考虑世界原点变换，考虑运行时
	UPROPERTY()
	FTransform StandPositionPosition = FTransform::Identity;

	FTransform GetStandPositionPosition() const { return StandPositionPosition; }

	UPROPERTY()
	UObject* AutoGenDialogueSequenceConfig;
	class UGenDialogueSequenceConfigBase* GetAutoGenDialogueSequenceConfig() const { return (UGenDialogueSequenceConfigBase*)AutoGenDialogueSequenceConfig; }

	UPROPERTY()
	TArray<UMovieSceneTrack*> AutoGenTracks;
	UPROPERTY()
	TArray<FGuid> AutoGenCameraGuids;
	UPROPERTY()
	TArray<FGuid> AutoGenCameraComponentGuids;
	UPROPERTY()
	TArray<FGuid> AutoGenSupplementLightGuids;

	FGuid FindOrAddPossessable(UObject* ObjectToPossess);
	FGuid CreateSpawnable(UObject* ObjectToSpawn);

	ULevelSequence* GetOwingLevelSequence() const;

	UPROPERTY()
	uint8 bIsNewCreated : 1;
	UPROPERTY()
	uint8 bIsNotSetStandPosition : 1;
public:

	// TODO：补全资源标签

	/**
	 * Extend the default ULevelSequence asset registry tags
	 */
	void ExtendAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override {}

	/**
	 * Extend the default ULevelSequence asset registry tag meta-data
	 */
	void ExtendAssetRegistryTagMetaData(TMap<FName, FAssetRegistryTagMetadata>& OutMetadata) const override {}
};
