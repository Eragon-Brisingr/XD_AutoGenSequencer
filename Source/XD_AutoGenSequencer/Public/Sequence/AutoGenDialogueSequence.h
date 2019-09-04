// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelSequence.h"
#include "GameFramework/Actor.h"
#include "AutoGenDialogueSequence.generated.h"

class UMovieSceneTrack;

/**
 * 
 */
UCLASS()
class XD_AUTOGENSEQUENCER_API UAutoGenDialogueSequence : public ULevelSequence
{
	GENERATED_BODY()
public:
	UAutoGenDialogueSequence();

	UPROPERTY()
	FTransform StandPositionPosition = FTransform::Identity;

	FTransform GetStandPositionPosition() const { return StandPositionPosition; }

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	class UPreviewDialogueSoundSequence* PreviewDialogueSoundSequence;

	UPROPERTY()
	class UAutoGenDialogueSequenceConfig* AutoGenDialogueSequenceConfig;

	UPROPERTY()
	TArray<UMovieSceneTrack*> AutoGenTracks;
	UPROPERTY()
	TArray<FGuid> AutoGenCameraGuids;
	UPROPERTY()
	TArray<FGuid> AutoGenCameraComponentGuids;

	FGuid FindOrAddPossessable(UObject* ObjectToPossess);
	using ULevelSequence::CreateSpawnable;

	UPROPERTY()
	uint8 bIsNewCreated : 1;
	UPROPERTY()
	uint8 bIsNotSetStandPosition : 1;
#endif
};
