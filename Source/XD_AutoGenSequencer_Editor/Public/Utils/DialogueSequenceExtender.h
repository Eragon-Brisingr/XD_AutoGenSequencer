// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FExtender;
class ISequencer;
class ISequencerModule;
class UAutoGenDialogueSystemData;
class UPreviewDialogueSoundSequence;
class UGenDialogueSequenceConfigBase;
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

	void BuildAutoGenToolbar(FToolBarBuilder &ToolBarBuilder);

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

	TWeakObjectPtr<UAutoGenDialogueSystemData> WeakAutoGenDialogueSystemData;
	UAutoGenDialogueSystemData* GetAutoGenDialogueSystemData() const;
	ULevelSequence* GetAutoGenDialogueSequence() const;
	UPreviewDialogueSoundSequence* GetPreviewDialogueSoundSequence() const;
	UGenDialogueSequenceConfigBase* GetAutoGenDialogueSequenceConfig() const;

	bool IsPreviewDialogueSequenceActived();
	bool IsAutoGenDialogueSequenceActived();
	void OpenPreviewDialogueSoundSequence();
	void OpenAutoGenDialogueSequence();

	void OnSequenceCreated(TSharedRef<ISequencer> InSequencer);
	void OnSequencerClosed(TSharedRef<ISequencer> InSequencer);

	void WhenAutoGenSequenceEditorOpened(UAutoGenDialogueSystemData* AutoGenDialogueSystemData);
	void WhenAutoGenSequenceEditorClosed();

	UWorld* GetEditorWorld() const;
	TSoftObjectPtr<ADialogueStandPositionTemplate> PreviewStandPositionTemplate;
	TArray<TSoftObjectPtr<ACharacter>> CachedSourceCharacterInstance;
	TMap<FName, TSoftObjectPtr<ACharacter>> CharacterNameInstanceMap;
	void DestroyPreviewStandPositionTemplate();
	void GeneratePreviewCharacters();
	//将生成的预览实例和定序器中的同步
	static void SyncSequenceInstanceReference(UAutoGenDialogueSystemData* AutoGenDialogueSystemData, const TMap<FName, TSoftObjectPtr<ACharacter>>& CharacterNameInstanceMap);

	void WhenStandTemplateInstanceChanged();

	void SetStandTemplateInstancePickable(bool Enable);
};
