﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FExtender;
class ISequencer;
class ISequencerModule;
class UAutoGenDialogueSequence;
class UPreviewDialogueSoundSequence;
class UAutoGenDialogueSequenceConfig;
class ADialogueStandPositionTemplate;
class UWorld;
class ACharacter;

/**
 * 
 */
class XD_AUTOGENSEQUENCER_EDITOR_API FDialogueSequenceExtender
{
public:	
	void Register(ISequencerModule& SequencerModule);

	void Unregister(ISequencerModule& SequencerModule);

	static FDialogueSequenceExtender& Get()
	{
		static FDialogueSequenceExtender DialogueSequenceExtender;
		return DialogueSequenceExtender;
	}
private:
	FDelegateHandle SequencerCreatedHandle;
	TSharedPtr<FExtender> SequencerToolbarExtender;
	TWeakPtr<ISequencer> WeakSequencer;

	TWeakObjectPtr<UAutoGenDialogueSequence> WeakAutoGenDialogueSequence;
	UAutoGenDialogueSequence* GetAutoGenDialogueSequence() const;
	UPreviewDialogueSoundSequence* GetPreviewDialogueSoundSequence() const;
	UGenDialogueSequenceConfigBase* GetAutoGenDialogueSequenceConfig() const;

	bool IsPreviewDialogueSequenceActived();
	bool IsAutoGenDialogueSequenceActived();

	void OnSequenceCreated(TSharedRef<ISequencer> InSequencer);
	void OnSequencerClosed(TSharedRef<ISequencer> InSequencer);

	UWorld* GetEditorWorld() const;
	TSoftObjectPtr<ADialogueStandPositionTemplate> PreviewStandPositionTemplate;
	TArray<TSoftObjectPtr<ACharacter>> CachedSourceCharacterInstance;
	TMap<FName, TSoftObjectPtr<ACharacter>> CharacterNameInstanceMap;
	void DestroyPreviewStandPositionTemplate();
	void GeneratePreviewCharacters();
	//将生成的预览实例和定序器中的同步
	static void SyncSequenceInstanceReference(UAutoGenDialogueSequence* AutoGenDialogueSequence, const TMap<FName, TSoftObjectPtr<ACharacter>>& CharacterNameInstanceMap);

	void WhenStandTemplateInstanceChanged();
};
