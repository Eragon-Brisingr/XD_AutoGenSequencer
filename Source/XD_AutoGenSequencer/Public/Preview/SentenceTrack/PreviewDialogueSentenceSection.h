// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovieSceneAudioSection.h"
#include "MovieSceneEvalTemplate.h"
#include "AutoGenDialogueSequenceConfig.h"
#include "PreviewDialogueSentenceSection.generated.h"

/**
 * 
 */
UCLASS()
class XD_AUTOGENSEQUENCER_API UPreviewDialogueSentenceSection : public UMovieSceneAudioSection
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	FDialogueSentenceEditData DialogueSentenceEditData;
};
