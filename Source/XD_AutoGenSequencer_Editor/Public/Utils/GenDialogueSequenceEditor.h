﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FExtender;
class ISequencer;
class ISequencerModule;
class UGenDialogueSequenceConfigBase;
class UPreviewDialogueSoundSequence;
class UGenDialogueSequenceConfigBase;
class ADialogueStandPositionTemplate;
class UWorld;
class ACharacter;
class ULevelSequence;

/**
 * 
 */
class XD_AUTOGENSEQUENCER_EDITOR_API FGenDialogueSequenceEditor
{
	friend class FEdMode_AutoGenSequence;
public:	
	void Register(ISequencerModule& SequencerModule);

	void BuildAutoGenToolbar(FToolBarBuilder &ToolBarBuilder);

	void Unregister(ISequencerModule& SequencerModule);

	void GenerateDialogueSequence();

	void GeneratePreviewSequence();

	static FGenDialogueSequenceEditor& Get();
private:
	FDelegateHandle SequencerCreatedHandle;
	TSharedPtr<FExtender> SequencerToolbarExtender;
	TWeakPtr<ISequencer> WeakSequencer;

	TWeakObjectPtr<UGenDialogueSequenceConfigBase> WeakGenDialogueSequenceConfig;
	UGenDialogueSequenceConfigBase* GetGenDialogueSequenceConfig() const;
	ULevelSequence* GetAutoGenDialogueSequence() const;
	UPreviewDialogueSoundSequence* GetPreviewDialogueSoundSequence() const;

	bool IsPreviewDialogueSequenceActived();
	bool IsAutoGenDialogueSequenceActived();
	void OpenPreviewDialogueSoundSequence();
	void OpenAutoGenDialogueSequence();

	void OnSequenceCreated(TSharedRef<ISequencer> InSequencer);
	void OnSequencerClosed(TSharedRef<ISequencer> InSequencer);

	void WhenAutoGenSequenceEditorOpened(UGenDialogueSequenceConfigBase* GenDialogueSequenceConfig);
	void WhenAutoGenSequenceEditorClosed();

	UWorld* GetEditorWorld() const;
	TSoftObjectPtr<ADialogueStandPositionTemplate> PreviewStandPositionTemplate;
	TArray<TSoftObjectPtr<ACharacter>> CachedCharacterInstances;
	TMap<FName, TSoftObjectPtr<ACharacter>> CharacterNameInstanceMap;
	TMap<FName, ACharacter*> GetCharacterNameInstanceMap() const;
	void DestroyPreviewStandPositionTemplate();
	void GeneratePreviewCharacters();
	//将生成的预览实例和定序器中的同步
	static void SyncSequenceInstanceReference(ULevelSequence* LevelSeqeunce, const TMap<FName, TSoftObjectPtr<ACharacter>>& CharacterNameInstanceMap);

	void WhenStandTemplateInstanceChanged();

	void UpdateStandTemplateInstanceState();

	static void OpenEditorForAsset(UObject* Asset);
};
