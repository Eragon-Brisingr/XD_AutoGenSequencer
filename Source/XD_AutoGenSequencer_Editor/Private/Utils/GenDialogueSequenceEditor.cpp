// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/GenDialogueSequenceEditor.h"

#include <ISequencerModule.h>
#include <Toolkits/AssetEditorToolkit.h>
#include <Framework/MultiBox/MultiBoxBuilder.h>
#include <Engine/World.h>
#include <GameFramework/Character.h>
#include <Components/ChildActorComponent.h>
#include <ScopedTransaction.h>
#include <Engine/Selection.h>
#include <LevelEditorViewport.h>
#include <Misc/MessageDialog.h>
#include <Editor.h>
#include <EditorModeManager.h>
#include <LevelSequence.h>

#include "Utils/AutoGenDialogueEditorStyle.h"
#include "Preview/Sequence/PreviewDialogueSoundSequence.h"
#include "Datas/DialogueStandPositionTemplate.h"
#include "Datas/GenDialogueSequenceConfigBase.h"
#include "Utils/EdMode_AutoGenSequence.h"
#include "Factory/AutoGenDialogueSequenceFactory.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

namespace FDialogueSequenceEditorHelper
{
	bool bIsInInnerSequenceSwitch = false;
}

void FGenDialogueSequenceEditor::Register(ISequencerModule& SequencerModule)
{
	SequencerCreatedHandle = SequencerModule.RegisterOnSequencerCreated(FOnSequencerCreated::FDelegate::CreateRaw(this, &FGenDialogueSequenceEditor::OnSequenceCreated));

	SequencerToolbarExtender = MakeShareable(new FExtender());
	SequencerToolbarExtender->AddToolBarExtension(
		"Base Commands",
		EExtensionHook::Before,
		nullptr,
		FToolBarExtensionDelegate::CreateRaw(this, &FGenDialogueSequenceEditor::BuildAutoGenToolbar));

	SequencerModule.GetToolBarExtensibilityManager()->AddExtender(SequencerToolbarExtender);
}

void FGenDialogueSequenceEditor::BuildAutoGenToolbar(FToolBarBuilder &ToolBarBuilder)
{
	ToolBarBuilder.AddToolBarButton(FUIAction(FExecuteAction::CreateLambda([=]()
			{
				if (UPreviewDialogueSoundSequence* PreviewDialogueSoundSequence = GetPreviewDialogueSoundSequence())
				{
					// TODO：考虑下修改配置的时候是否要强制生成预览导轨
// 					if (!IsPreviewDialogueSequenceActived())
// 					{
// 						OpenPreviewDialogueSoundSequence();
// 					}
					OpenEditorForAsset(GetGenDialogueSequenceConfig());
				}
			}),
		FCanExecuteAction(),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateLambda([=]()
			{
				return WeakGenDialogueSequenceConfig.IsValid();
			})), NAME_None,
		LOCTEXT("打开对白配置", "打开对白配置"),
		LOCTEXT("打开对白配置", "打开对白配置"),
		FAutoGenDialogueEditorStyle::Get().GetConfigIcon());
	ToolBarBuilder.AddToolBarButton(FUIAction(FExecuteAction::CreateLambda([=]()
			{
				if (UPreviewDialogueSoundSequence* PreviewDialogueSoundSequence = GetPreviewDialogueSoundSequence())
				{
					OpenPreviewDialogueSoundSequence();
				}
			}),
		FCanExecuteAction::CreateLambda([=]()
			{
				return IsAutoGenDialogueSequenceActived();
			}),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateLambda([=]()
			{
				return WeakGenDialogueSequenceConfig.IsValid();
			})), NAME_None,
		LOCTEXT("打开预览序列", "打开预览序列"),
		LOCTEXT("打开预览序列", "打开预览序列"),
		FAutoGenDialogueEditorStyle::Get().GetPreviewIcon());
	ToolBarBuilder.AddToolBarButton(FUIAction(FExecuteAction::CreateLambda([=]()
		{
			OpenAutoGenDialogueSequence();
		}),
		FCanExecuteAction::CreateLambda([=]()
			{
				return IsPreviewDialogueSequenceActived();
			}),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateLambda([=]()
			{
				return WeakGenDialogueSequenceConfig.IsValid();
			})), NAME_None,
		LOCTEXT("打开对白序列", "打开对白序列"),
		LOCTEXT("打开对白序列", "打开对白序列"),
		FAutoGenDialogueEditorStyle::Get().GetDialogueIcon());
	ToolBarBuilder.AddToolBarButton(FUIAction(FExecuteAction::CreateLambda([=]()
			{
				GenerateDialogueSequence();
			}),
		FCanExecuteAction::CreateLambda([]()
			{
				return true;
			}),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateLambda([=]()
			{
				return WeakGenDialogueSequenceConfig.IsValid() && IsPreviewDialogueSequenceActived();
			})), NAME_None,
		LOCTEXT("生成最终对话序列", "生成最终对话序列"),
		LOCTEXT("生成最终对话序列", "生成最终对话序列"),
		FAutoGenDialogueEditorStyle::Get().GetGenerateIcon());

	ToolBarBuilder.AddToolBarButton(FUIAction(FExecuteAction::CreateLambda([=]()
			{
				if (PreviewStandPositionTemplate.IsValid())
				{
					DestroyPreviewStandPositionTemplate();
				}
				GeneratePreviewCharacters();
			}),
		FCanExecuteAction::CreateLambda([]()
			{
				return true;
			}),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateLambda([=]()
			{
				return WeakGenDialogueSequenceConfig.IsValid();
			}), EUIActionRepeatMode::RepeatEnabled), NAME_None,
		LOCTEXT("刷新模板显示", "刷新模板显示"),
		LOCTEXT("刷新模板显示", "刷新模板显示"),
		FAutoGenDialogueEditorStyle::Get().GetStandpositionIcon());

	ToolBarBuilder.AddToolBarButton(FUIAction(FExecuteAction::CreateLambda([=]()
			{
				FEditorModeTools& EditorModeTools = GLevelEditorModeTools();
				if (EditorModeTools.IsModeActive(FEdMode_AutoGenSequence::ID))
				{
					EditorModeTools.DeactivateMode(FEdMode_AutoGenSequence::ID);
				}
				else
				{
					EditorModeTools.ActivateMode(FEdMode_AutoGenSequence::ID);
				}
			}),
		FCanExecuteAction::CreateLambda([]()
			{
				return true;
			}),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateLambda([=]()
			{
				return WeakGenDialogueSequenceConfig.IsValid();
			}), EUIActionRepeatMode::RepeatEnabled), NAME_None,
		LOCTEXT("切换辅助绘制模式", "切换辅助绘制模式"),
		LOCTEXT("切换辅助绘制模式", "切换辅助绘制模式"),
		FSlateIcon());

	ToolBarBuilder.AddSeparator("Auto Gen Dialogue");
}

void FGenDialogueSequenceEditor::Unregister(ISequencerModule& SequencerModule)
{
	SequencerModule.UnregisterSequenceEditor(SequencerCreatedHandle);
	SequencerModule.GetToolBarExtensibilityManager()->RemoveExtender(SequencerToolbarExtender);
}

void FGenDialogueSequenceEditor::GenerateDialogueSequence()
{
	UGenDialogueSequenceConfigBase* GenDialogueSequenceConfig = GetGenDialogueSequenceConfig();
	TArray<FText> ErrorMessages;
	if (!GenDialogueSequenceConfig->IsConfigValid(ErrorMessages))
	{
		FText ErrorMessage = FText::GetEmpty();
		for (const FText& Error : ErrorMessages)
		{
			ErrorMessage = FText::Format(LOCTEXT("Error Append", "{0}\n{1}"), ErrorMessage, Error);
		}
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("预览导轨生成报错", "配置中存在问题，无法生成{0}"), ErrorMessage));
		return;
	}

	if (!PreviewStandPositionTemplate.IsValid())
	{
		GeneratePreviewCharacters();
	}
	GenDialogueSequenceConfig->Generate(WeakSequencer.Pin().ToSharedRef(), GetEditorWorld(), GetCharacterNameInstanceMap());

	TGuardValue<bool> InnerSequenceSwitchGuard(FDialogueSequenceEditorHelper::bIsInInnerSequenceSwitch, true);
	//不知道怎么直接刷新，临时切换下来刷新 ISequencer::NotifyMovieSceneDataChanged
	OpenEditorForAsset(GetPreviewDialogueSoundSequence());
	OpenEditorForAsset(GetAutoGenDialogueSequence());
}

void FGenDialogueSequenceEditor::GeneratePreviewSequence()
{
	UGenDialogueSequenceConfigBase* GenDialogueSequenceConfig = GetGenDialogueSequenceConfig();
	TArray<FText> ErrorMessages;
	if (!GenDialogueSequenceConfig->IsConfigValid(ErrorMessages))
	{
		FText ErrorMessage = FText::GetEmpty();
		for (const FText& Error : ErrorMessages)
		{
			ErrorMessage = FText::Format(LOCTEXT("Error Append", "{0}\n{1}"), ErrorMessage, Error);
		}
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("预览导轨生成报错", "配置中存在问题，无法生成{0}"), ErrorMessage));
		return;
	}

	if (!PreviewStandPositionTemplate.IsValid())
	{
		GeneratePreviewCharacters();
	}
	GenDialogueSequenceConfig->GeneratePreview(GetCharacterNameInstanceMap());

	TGuardValue<bool> InnerSequenceSwitchGuard(FDialogueSequenceEditorHelper::bIsInInnerSequenceSwitch, true);
	//不知道怎么直接刷新，临时切换下来刷新 ISequencer::NotifyMovieSceneDataChanged
	OpenEditorForAsset(GetAutoGenDialogueSequence());
	OpenEditorForAsset(GetPreviewDialogueSoundSequence());
}

UGenDialogueSequenceConfigBase* FGenDialogueSequenceEditor::GetGenDialogueSequenceConfig() const
{
	return WeakGenDialogueSequenceConfig.Get();
}

ULevelSequence* FGenDialogueSequenceEditor::GetAutoGenDialogueSequence() const
{
	return WeakGenDialogueSequenceConfig->GetOwingLevelSequence();
}

UPreviewDialogueSoundSequence* FGenDialogueSequenceEditor::GetPreviewDialogueSoundSequence() const
{
	return WeakGenDialogueSequenceConfig.IsValid() ? WeakGenDialogueSequenceConfig.Get()->PreviewDialogueSequence : nullptr;
}

bool FGenDialogueSequenceEditor::IsPreviewDialogueSequenceActived()
{
	return WeakSequencer.Pin()->GetFocusedMovieSceneSequence() == GetPreviewDialogueSoundSequence();
}

bool FGenDialogueSequenceEditor::IsAutoGenDialogueSequenceActived()
{
	return WeakSequencer.Pin()->GetFocusedMovieSceneSequence() == GetAutoGenDialogueSequence();
}

void FGenDialogueSequenceEditor::OpenPreviewDialogueSoundSequence()
{
	TGuardValue<bool> InnerSequenceSwitchGuard(FDialogueSequenceEditorHelper::bIsInInnerSequenceSwitch, true);
	OpenEditorForAsset(GetPreviewDialogueSoundSequence());
	SetStandTemplateInstancePickable(false);
}

void FGenDialogueSequenceEditor::OpenAutoGenDialogueSequence()
{
	TGuardValue<bool> InnerSequenceSwitchGuard(FDialogueSequenceEditorHelper::bIsInInnerSequenceSwitch, true);
	OpenEditorForAsset(GetAutoGenDialogueSequence());
	SetStandTemplateInstancePickable(true);
}

void FGenDialogueSequenceEditor::OnSequenceCreated(TSharedRef<ISequencer> InSequencer)
{
	if (WeakSequencer.IsValid())
	{
		WeakSequencer.Pin()->OnCloseEvent().RemoveAll(this);
	}
	InSequencer->OnCloseEvent().AddRaw(this, &FGenDialogueSequenceEditor::OnSequencerClosed);
	WeakSequencer = InSequencer;
	if (!FDialogueSequenceEditorHelper::bIsInInnerSequenceSwitch)
	{
		ULevelSequence* LevelSequence = Cast<ULevelSequence>(InSequencer->GetFocusedMovieSceneSequence());
		if (!LevelSequence)
		{
			return;
		}

		UGenDialogueSequenceConfigContainer* GenDialogueSequenceConfigContainer = LevelSequence->FindMetaData<UGenDialogueSequenceConfigContainer>();
		if (GenDialogueSequenceConfigContainer && GenDialogueSequenceConfigContainer->GenDialogueSequenceConfig)
		{
			WhenAutoGenSequenceEditorOpened(GenDialogueSequenceConfigContainer->GenDialogueSequenceConfig);
		}
		else
		{
			WhenAutoGenSequenceEditorClosed();
		}
	}

	if (FEdMode_AutoGenSequence* EdMode_AutoGenSequence = StaticCast<FEdMode_AutoGenSequence*>(GLevelEditorModeTools().GetActiveMode(FEdMode_AutoGenSequence::ID)))
	{
		EdMode_AutoGenSequence->WeakSequencer = WeakSequencer;
	}
}

void FGenDialogueSequenceEditor::OnSequencerClosed(TSharedRef<ISequencer> InSequencer)
{
	if (!FDialogueSequenceEditorHelper::bIsInInnerSequenceSwitch && WeakGenDialogueSequenceConfig.IsValid())
	{
		WhenAutoGenSequenceEditorClosed();
	}
}

void FGenDialogueSequenceEditor::WhenAutoGenSequenceEditorOpened(UGenDialogueSequenceConfigBase* InGenDialogueSequenceConfig)
{
	UGenDialogueSequenceConfigBase* OldGenDialogueSequenceConfig = WeakGenDialogueSequenceConfig.Get();
	WeakGenDialogueSequenceConfig = InGenDialogueSequenceConfig;

	if (InGenDialogueSequenceConfig->bIsNewCreated)
	{
		InGenDialogueSequenceConfig->bIsNewCreated = false;
	}
	
	if (OldGenDialogueSequenceConfig != InGenDialogueSequenceConfig)
	{
		GeneratePreviewCharacters();
		SetStandTemplateInstancePickable(true);

		if (!InGenDialogueSequenceConfig->PreviewDialogueSequence->HasPreviewData())
		{
			OpenEditorForAsset(GetGenDialogueSequenceConfig());
			// 直接开不行，延迟一帧
			GEditor->GetTimerManager().Get().SetTimerForNextTick([=]()
				{
					OpenPreviewDialogueSoundSequence();
				});
		}
	}
}

void FGenDialogueSequenceEditor::WhenAutoGenSequenceEditorClosed()
{
	WeakSequencer = nullptr;
	WeakGenDialogueSequenceConfig = nullptr;
	if (PreviewStandPositionTemplate.IsValid())
	{
		DestroyPreviewStandPositionTemplate();
	}
	GLevelEditorModeTools().DeactivateMode(FEdMode_AutoGenSequence::ID);
}

UWorld* FGenDialogueSequenceEditor::GetEditorWorld() const
{
	return GEditor->GetEditorWorldContext().World();
}

TMap<FName, ACharacter*> FGenDialogueSequenceEditor::GetCharacterNameInstanceMap() const
{
	TMap<FName, ACharacter*> NameInstanceMap;
	for (const TPair<FName, TSoftObjectPtr<ACharacter>>& Pair : CharacterNameInstanceMap)
	{
		ACharacter* Character = Pair.Value.Get();
		if (Character)
		{
			NameInstanceMap.Add(Pair.Key, Character);
		}
	}
	return NameInstanceMap;
}

void FGenDialogueSequenceEditor::DestroyPreviewStandPositionTemplate()
{
	for (const TPair<FName, TSoftObjectPtr<ACharacter>>& Pair :  CharacterNameInstanceMap)
	{
		if (ACharacter* Character = Pair.Value.Get())
		{
			Character->Destroy();
		}
	}
	CharacterNameInstanceMap.Empty();

	for (const TSoftObjectPtr<ACharacter>& CharacterPtr : CachedCharacterInstances)
	{
		if (ACharacter* Character = CharacterPtr.Get())
		{
			Character->SetIsTemporarilyHiddenInEditor(false);
			Character->ForEachAttachedActors([=](AActor* Actor)
				{
					Actor->SetIsTemporarilyHiddenInEditor(false);
					return true;
				});
		}
	}
	CachedCharacterInstances.Empty();

	if (PreviewStandPositionTemplate.IsValid())
	{
		PreviewStandPositionTemplate->Destroy();
	}
	PreviewStandPositionTemplate = nullptr;
}

void FGenDialogueSequenceEditor::GeneratePreviewCharacters()
{
	FEditorScriptExecutionGuard EditorScriptExecutionGuard;

	DestroyPreviewStandPositionTemplate();

	UGenDialogueSequenceConfigBase* DialogueSequenceConfig = GetGenDialogueSequenceConfig();

	if (TSubclassOf<ADialogueStandPositionTemplate> StandTemplateType = DialogueSequenceConfig->DialogueStationTemplate)
	{
		FTransform SpawnTransform = DialogueSequenceConfig->StandPositionPosition;
		if (DialogueSequenceConfig->bIsNotSetStandPosition)
		{
			DialogueSequenceConfig->bIsNotSetStandPosition = false;
			if (const FLevelEditorViewportClient* client = static_cast<const FLevelEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient()))
			{
				FRotator ViewRotation = client->GetViewRotation();
				FRotator Rotatoion = FRotator::ZeroRotator;
				Rotatoion.Yaw = ViewRotation.Yaw;
				SpawnTransform.SetRotation(Rotatoion.Quaternion());
				SpawnTransform.SetLocation(client->GetViewLocation() + ViewRotation.Vector() * 500.f);
				DialogueSequenceConfig->StandPositionPosition = SpawnTransform;
			}
		}

		const TArray<FDialogueCharacterData>& DialogueCharacterDatas = DialogueSequenceConfig->DialogueCharacterDatas;
		ADialogueStandPositionTemplate* StandPositionTemplate = GetEditorWorld()->SpawnActorDeferred<ADialogueStandPositionTemplate>(StandTemplateType, SpawnTransform);
		{
			StandPositionTemplate->StandPositions.SetNumZeroed(DialogueCharacterDatas.Num());
			StandPositionTemplate->bSpawnedPreviewCharacter = true;
			StandPositionTemplate->FinishSpawning(DialogueSequenceConfig->StandPositionPosition, true);
			StandPositionTemplate->SetFlags(RF_Transient);
			GEditor->GetSelectedActors()->DeselectAll();
			GEditor->SelectActor(StandPositionTemplate, true, false, false, true);
			StandPositionTemplate->InvalidateLightingCache();
			StandPositionTemplate->PostEditMove(true);
			GEditor->NoteSelectionChange();
		}
		PreviewStandPositionTemplate = StandPositionTemplate;

		TArray<FDialogueStandPosition>& StandPositions = StandPositionTemplate->StandPositions;
		for (int32 Idx = 0; Idx < StandPositions.Num() && Idx < DialogueCharacterDatas.Num(); ++Idx)
		{
			const FDialogueCharacterData& DialogueCharacterData = DialogueCharacterDatas[Idx];
			FDialogueStandPosition& StandPosition = StandPositions[Idx];
			const FName& CharacterName = DialogueCharacterData.NameOverride;
			check(!CharacterNameInstanceMap.Contains(CharacterName));

			UChildActorComponent* ChildActorComponent = StandPositionTemplate->CreateChildActorComponent();
			StandPosition.PreviewCharacterInstance = ChildActorComponent;

			ACharacter* PreviewCharacter = nullptr;
			if (ACharacter* Character = DialogueCharacterData.InstanceOverride.Get())
			{
				// UChildActorComponent::ChildActorTemplate
				AActor*& ChildActorTemplate = *UChildActorComponent::StaticClass()->FindPropertyByName(TEXT("ChildActorTemplate"))->ContainerPtrToValuePtr<AActor*>(ChildActorComponent);
				ChildActorTemplate = DuplicateObject(Character, ChildActorComponent);
				ChildActorComponent->SetChildActorClass(Character->GetClass());
				PreviewCharacter = Cast<ACharacter>(ChildActorComponent->GetChildActor());

				Character->SetIsTemporarilyHiddenInEditor(true);
				Character->ForEachAttachedActors([=](AActor* Actor)
					{
						Actor->SetIsTemporarilyHiddenInEditor(true);
						return true;
					});
				CachedCharacterInstances.Add(Character);
			}
			else
			{
				TSubclassOf<ACharacter> PreviewCharacterType = DialogueCharacterData.TypeOverride;
				PreviewCharacterType = PreviewCharacterType ? PreviewCharacterType : StandPosition.PreviewCharacter;
				PreviewCharacterType = PreviewCharacterType ? PreviewCharacterType : StandPositionTemplate->PreviewCharacter;

				check(PreviewCharacterType);
				ChildActorComponent->SetChildActorClass(PreviewCharacterType);
				PreviewCharacter = Cast<ACharacter>(ChildActorComponent->GetChildActor());
			}

			check(PreviewCharacter);
			PreviewCharacter->SetFlags(RF_Transient);
			PreviewCharacter->SetActorLabel(CharacterName.ToString());
			if (PreviewCharacter->Rename(*CharacterName.ToString(), nullptr, REN_Test))
			{
				PreviewCharacter->Rename(*CharacterName.ToString(), nullptr);
			}
			PreviewCharacter->SetActorRelativeLocation(FVector(0.f, 0.f, PreviewCharacter->GetDefaultHalfHeight()));

			CharacterNameInstanceMap.Add(CharacterName, PreviewCharacter);
			ChildActorComponent->SetRelativeTransform(DialogueCharacterData.PositionOverride);
			StandPosition.StandPosition = DialogueCharacterData.PositionOverride;

			SyncSequenceInstanceReference(DialogueSequenceConfig->PreviewDialogueSequence, CharacterNameInstanceMap);
			SyncSequenceInstanceReference(DialogueSequenceConfig->GetOwingLevelSequence(), CharacterNameInstanceMap);
		}

		StandPositionTemplate->OnInstanceChanged.BindRaw(this, &FGenDialogueSequenceEditor::WhenStandTemplateInstanceChanged);
	}
}

void FGenDialogueSequenceEditor::SyncSequenceInstanceReference(ULevelSequence* LevelSeqeunce, const TMap<FName, TSoftObjectPtr<ACharacter>>& CharacterNameInstanceMap)
{
	UMovieScene* MovieScene = LevelSeqeunce->GetMovieScene();

	for (const TPair<FName, TSoftObjectPtr<ACharacter>>& Pair : CharacterNameInstanceMap)
	{
		if (FMovieScenePossessable* MovieScenePossessable = MovieScene->FindPossessable([=](const FMovieScenePossessable& E) {return E.GetName() == Pair.Key.ToString(); }))
		{
			ACharacter* SpeakerInstance = Pair.Value.Get();
			check(SpeakerInstance);
			FGuid BindingGuid = MovieScenePossessable->GetGuid();
			LevelSeqeunce->UnbindPossessableObjects(BindingGuid);
			LevelSeqeunce->BindPossessableObject(MovieScenePossessable->GetGuid(), *SpeakerInstance, SpeakerInstance);
		}
	}
}

void FGenDialogueSequenceEditor::WhenStandTemplateInstanceChanged()
{
	ADialogueStandPositionTemplate* StandPositionTemplate = PreviewStandPositionTemplate.Get();
	UGenDialogueSequenceConfigBase* AutoGenDialogueSequenceConfig = GetGenDialogueSequenceConfig();
	StandPositionTemplate->Modify();
	AutoGenDialogueSequenceConfig->Modify();

	AutoGenDialogueSequenceConfig->SyncInstanceData(StandPositionTemplate);
	GetGenDialogueSequenceConfig()->StandPositionPosition = StandPositionTemplate->GetActorTransform();
}

void FGenDialogueSequenceEditor::SetStandTemplateInstancePickable(bool Enable)
{
	if (PreviewStandPositionTemplate.IsValid() == false)
	{
		return;
	}

	UProperty* ParentComponentProperty = AActor::StaticClass()->FindPropertyByName(TEXT("ParentComponent"));
	for (TPair<FName, TSoftObjectPtr<ACharacter>>& Pair : CharacterNameInstanceMap)
	{
		if (ACharacter* Talker = Pair.Value.Get())
		{
			TWeakObjectPtr<UChildActorComponent>& ParentComponent = *ParentComponentProperty->ContainerPtrToValuePtr<TWeakObjectPtr<UChildActorComponent>>(Talker);
			if (Enable)
			{
				ParentComponent = nullptr;
			}
			else
			{
				ParentComponent = *PreviewStandPositionTemplate->PreviewCharacters.FindByPredicate([&](UChildActorComponent* E) {return E->GetChildActor() == Talker; });
			}
		}
	}
	
}

void FGenDialogueSequenceEditor::OpenEditorForAsset(UObject* Asset)
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	AssetEditorSubsystem->OpenEditorForAsset(Asset);
}

#undef LOCTEXT_NAMESPACE
