// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoGenDialogueSystemData.h"
#include "LevelSequence.h"
#include "GenDialogueSequenceConfigBase.h"
#include "PreviewDialogueSoundSequence.h"

class ULevelSequenceUtils : public ULevelSequence
{
public:
	static FGuid FindOrAddPossessable(ULevelSequence* LevelSequence, UObject* ObjectToPossess)
	{
		return StaticCast<ULevelSequenceUtils*>(LevelSequence)->CreatePossessable(ObjectToPossess);
	}

	static FGuid AddSpawnable(ULevelSequence* LevelSequence, UObject* ObjectToSpawn)
	{
		return StaticCast<ULevelSequenceUtils*>(LevelSequence)->ULevelSequence::CreateSpawnable(ObjectToSpawn);
	}
};

FGuid UAutoGenDialogueSystemData::FindOrAddPossessable(UObject* ObjectToPossess)
{
	return ULevelSequenceUtils::FindOrAddPossessable(GetOwingLevelSequence(), ObjectToPossess);
}

FGuid UAutoGenDialogueSystemData::CreateSpawnable(UObject* ObjectToSpawn)
{
	return ULevelSequenceUtils::AddSpawnable(GetOwingLevelSequence(), ObjectToSpawn);
}

ULevelSequence* UAutoGenDialogueSystemData::GetOwingLevelSequence() const
{
	return GetTypedOuter<ULevelSequence>();
}

bool UAutoGenDialogueSystemData::HasPreviewData() const
{
	return AutoGenDialogueSequenceConfig->PreviewDialogueSoundSequence->HasPreviewData();
}
