// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Sections/MovieSceneAudioSection.h>
#include <Evaluation/MovieSceneEvalTemplate.h>
#include "Datas/AutoGenDialogueType.h"
#include "PreviewDialogueSentenceSection.generated.h"

/**
 * 
 */
UCLASS()
class XD_AUTOGENSEQUENCER_EDITOR_API UPreviewDialogueSentenceSection : public UMovieSceneAudioSection
{
	GENERATED_BODY()
public:
	UPreviewDialogueSentenceSection();

	UPROPERTY(EditAnywhere)
	FDialogueSentenceEditData DialogueSentenceEditData;
};
