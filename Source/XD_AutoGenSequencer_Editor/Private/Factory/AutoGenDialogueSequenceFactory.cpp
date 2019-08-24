// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoGenDialogueSequenceFactory.h"
#include "AssetTypeCategories.h"
#include "MovieSceneToolsProjectSettings.h"
#include "FrameRate.h"
#include "PreviewDialogueSoundSequence.h"
#include "AutoGenDialogueSequence.h"
#include "PreviewDialogueSentenceTrack.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

UAutoGenDialogueSequenceFactory::UAutoGenDialogueSequenceFactory()
{
	SupportedClass = UAutoGenDialogueSequence::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UAutoGenDialogueSequenceFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	UAutoGenDialogueSequence* NewLevelSequence = NewObject<UAutoGenDialogueSequence>(InParent, Name, Flags | RF_Transactional);
	NewLevelSequence->Initialize();

	// Set up some sensible defaults
	const UMovieSceneToolsProjectSettings* ProjectSettings = GetDefault<UMovieSceneToolsProjectSettings>();
	FFrameRate TickResolution = NewLevelSequence->GetMovieScene()->GetTickResolution();
	NewLevelSequence->GetMovieScene()->SetPlaybackRange((ProjectSettings->DefaultStartTime*TickResolution).FloorToFrame(), (ProjectSettings->DefaultDuration*TickResolution).FloorToFrame().Value);

	NewLevelSequence->PreviewDialogueSoundSequence = NewObject<UPreviewDialogueSoundSequence>(NewLevelSequence, GET_MEMBER_NAME_CHECKED(UAutoGenDialogueSequence, PreviewDialogueSoundSequence), Flags | RF_Transactional);
	{
		NewLevelSequence->PreviewDialogueSoundSequence->Initialize();
	}
	NewLevelSequence->AutoGenDialogueSequenceConfig = NewObject<UAutoGenDialogueSequenceConfig>(NewLevelSequence, GET_MEMBER_NAME_CHECKED(UAutoGenDialogueSequence, AutoGenDialogueSequenceConfig), Flags | RF_Transactional);

	NewLevelSequence->bIsNewCreated = true;
	NewLevelSequence->bIsNotSetStandPosition = true;

	return NewLevelSequence;
}

UObject* UAutoGenDialogueSequenceFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FactoryCreateNew(InClass, InParent, InName, Flags, Context, Warn, NAME_None);
}

FText UAutoGenDialogueSequenceFactory::GetDisplayName() const
{
	return LOCTEXT("创建对话定序器", "对话定序器");
}

uint32 UAutoGenDialogueSequenceFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Animation;
}

UPreviewDialogueSequenceFactory::UPreviewDialogueSequenceFactory()
{
	SupportedClass = UPreviewDialogueSoundSequence::StaticClass();
	bCreateNew = false;
	bEditAfterNew = true;
}

UObject* UPreviewDialogueSequenceFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FactoryCreateNew(InClass, InParent, InName, Flags, Context, Warn, NAME_None);
}

UObject* UPreviewDialogueSequenceFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	UPreviewDialogueSoundSequence* NewLevelSequence = NewObject<UPreviewDialogueSoundSequence>(InParent, Name, Flags | RF_Transactional);
	NewLevelSequence->Initialize();

	// Set up some sensible defaults
	const UMovieSceneToolsProjectSettings* ProjectSettings = GetDefault<UMovieSceneToolsProjectSettings>();

	FFrameRate TickResolution = NewLevelSequence->GetMovieScene()->GetTickResolution();
	NewLevelSequence->GetMovieScene()->SetPlaybackRange((ProjectSettings->DefaultStartTime*TickResolution).FloorToFrame(), (ProjectSettings->DefaultDuration*TickResolution).FloorToFrame().Value);

	return NewLevelSequence;
}

FText UPreviewDialogueSequenceFactory::GetDisplayName() const
{
	return LOCTEXT("创建对话预览定序器", "对话预览定序器");
}

uint32 UPreviewDialogueSequenceFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Animation;
}

#undef LOCTEXT_NAMESPACE
