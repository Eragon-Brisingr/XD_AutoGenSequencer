// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoGenDialogueSequenceFactory.h"
#include "MovieSceneToolsProjectSettings.h"
#include "FrameRate.h"
#include "PreviewDialogueSoundSequence.h"
#include "AutoGenDialogueSequence.h"
#include "PreviewDialogueSentenceTrack.h"
#include "AutoGenDialogueSequenceConfig.h"
#include "XD_AutoGenSequencer_Editor.h"

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
	return LOCTEXT("创建对白定序器", "对白定序器");
}

uint32 UAutoGenDialogueSequenceFactory::GetMenuCategories() const
{
	return FXD_AutoGenSequencer_EditorModule::AutoGenDialogueSequence_AssetCategory;
}

#undef LOCTEXT_NAMESPACE
