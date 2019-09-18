// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueSequenceExtender.h"

#include "GenDialogueSequenceConfigBase.h"
#include "LevelSequence.h"
#include "AutoGenDialogueSystemData.h"
#include "PreviewDialogueSoundSequence.h"
#include "DialogueStandPositionTemplate.h"

#include "ISequencerModule.h"
#include "AutoGenDialogueEditorStyle.h"
#include "AssetEditorToolkit.h"
#include "MultiBoxBuilder.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/ChildActorComponent.h"
#include "ScopedTransaction.h"
#include "Engine/Selection.h"
#include "LevelEditorViewport.h"
#include "MessageDialog.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

namespace FDialogueSequenceEditorHelper
{
	bool bIsInInnerSequenceSwitch = false;
}

void FDialogueSequenceExtender::Register(ISequencerModule& SequencerModule)
{
	SequencerCreatedHandle = SequencerModule.RegisterOnSequencerCreated(FOnSequencerCreated::FDelegate::CreateRaw(this, &FDialogueSequenceExtender::OnSequenceCreated));

	SequencerToolbarExtender = MakeShareable(new FExtender());
	SequencerToolbarExtender->AddToolBarExtension(
		"Base Commands",
		EExtensionHook::Before,
		nullptr,
		FToolBarExtensionDelegate::CreateRaw(this, &FDialogueSequenceExtender::BuildAutoGenToolbar));

	SequencerModule.GetToolBarExtensibilityManager()->AddExtender(SequencerToolbarExtender);
}

void FDialogueSequenceExtender::BuildAutoGenToolbar(FToolBarBuilder &ToolBarBuilder)
{
	ToolBarBuilder.AddToolBarButton(FUIAction(FExecuteAction::CreateLambda([=]()
			{
				if (UPreviewDialogueSoundSequence* PreviewDialogueSoundSequence = GetPreviewDialogueSoundSequence())
				{
					if (!IsPreviewDialogueSequenceActived())
					{
						OpenPreviewDialogueSoundSequence();
					}
					FAssetEditorManager::Get().OpenEditorForAsset(GetAutoGenDialogueSequenceConfig());
				}
			}),
		FCanExecuteAction(),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateLambda([=]()
			{
				return WeakAutoGenDialogueSystemData.IsValid();
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
				return WeakAutoGenDialogueSystemData.IsValid();
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
				return WeakAutoGenDialogueSystemData.IsValid();
			})), NAME_None,
		LOCTEXT("打开对白序列", "打开对白序列"),
		LOCTEXT("打开对白序列", "打开对白序列"),
		FAutoGenDialogueEditorStyle::Get().GetDialogueIcon());
	ToolBarBuilder.AddToolBarButton(FUIAction(FExecuteAction::CreateLambda([=]()
			{
				UGenDialogueSequenceConfigBase* Config = GetAutoGenDialogueSequenceConfig();
				if (IsPreviewDialogueSequenceActived())
				{
					UPreviewDialogueSoundSequence* PreviewDialogueSoundSequence = GetPreviewDialogueSoundSequence();

					TArray<FText> ErrorMessages;
					if (!Config->IsConfigValid(ErrorMessages))
					{
						FText ErrorMessage = FText::GetEmpty();
						for (const FText& Error : ErrorMessages)
						{
							ErrorMessage = FText::Format(LOCTEXT("Error Append", "{0}\n{1}"), ErrorMessage, Error);
						}
						FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("预览导轨生成报错", "配置中存在问题，无法生成{0}"), ErrorMessage));
						return;
					}

					GeneratePreviewCharacters();
					Config->GeneratePreview();

					TGuardValue<bool> InnerSequenceSwitchGuard(FDialogueSequenceEditorHelper::bIsInInnerSequenceSwitch, true);
					//不知道怎么直接刷新，临时切换下来刷新 ISequencer::NotifyMovieSceneDataChanged
					FAssetEditorManager::Get().OpenEditorForAsset(GetAutoGenDialogueSequence());
					FAssetEditorManager::Get().OpenEditorForAsset(PreviewDialogueSoundSequence);
				}
				else if (IsAutoGenDialogueSequenceActived())
				{
					TArray<FText> ErrorMessages;
					if (!Config->IsConfigValid(ErrorMessages))
					{
						FText ErrorMessage = FText::GetEmpty();
						for (const FText& Error : ErrorMessages)
						{
							ErrorMessage = FText::Format(LOCTEXT("Error Append", "{0}\n{1}"), ErrorMessage, Error);
						}
						FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("预览导轨生成报错", "配置中存在问题，无法生成{0}"), ErrorMessage));
						return;
					}

					UAutoGenDialogueSystemData* AutoGenDialogueSystemData = GetAutoGenDialogueSystemData();
					if (!PreviewStandPositionTemplate.IsValid())
					{
						GeneratePreviewCharacters();
					}

					UGenDialogueSequenceConfigBase* GenDialogueSequenceConfig = AutoGenDialogueSystemData->GetAutoGenDialogueSequenceConfig();
					GenDialogueSequenceConfig->Generate(WeakSequencer.Pin().ToSharedRef(), GetEditorWorld(), CharacterNameInstanceMap, *AutoGenDialogueSystemData);

					TGuardValue<bool> InnerSequenceSwitchGuard(FDialogueSequenceEditorHelper::bIsInInnerSequenceSwitch, true);
					//不知道怎么直接刷新，临时切换下来刷新 ISequencer::NotifyMovieSceneDataChanged
					FAssetEditorManager::Get().OpenEditorForAsset(GetPreviewDialogueSoundSequence());
					FAssetEditorManager::Get().OpenEditorForAsset(GetAutoGenDialogueSequence());
				}
			}),
		FCanExecuteAction::CreateLambda([]()
			{
				return true;
			}),
		FIsActionChecked(),
		FIsActionButtonVisible::CreateLambda([=]()
			{
				return WeakAutoGenDialogueSystemData.IsValid();
			})), NAME_None,
		LOCTEXT("生成", "生成"),
		LOCTEXT("生成", "生成"),
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
				return WeakAutoGenDialogueSystemData.IsValid();
			}), EUIActionRepeatMode::RepeatEnabled), NAME_None,
		LOCTEXT("刷新模板显示", "刷新模板显示"),
		LOCTEXT("刷新模板显示", "刷新模板显示"),
		FAutoGenDialogueEditorStyle::Get().GetStandpositionIcon());

	ToolBarBuilder.AddSeparator("Auto Gen Dialogue");
}

void FDialogueSequenceExtender::Unregister(ISequencerModule& SequencerModule)
{
	SequencerModule.UnregisterSequenceEditor(SequencerCreatedHandle);
	SequencerModule.GetToolBarExtensibilityManager()->RemoveExtender(SequencerToolbarExtender);
}

UAutoGenDialogueSystemData* FDialogueSequenceExtender::GetAutoGenDialogueSystemData() const
{
	return WeakAutoGenDialogueSystemData.Get();
}

ULevelSequence* FDialogueSequenceExtender::GetAutoGenDialogueSequence() const
{
	return WeakAutoGenDialogueSystemData->GetOwingLevelSequence();
}

UPreviewDialogueSoundSequence* FDialogueSequenceExtender::GetPreviewDialogueSoundSequence() const
{
	return WeakAutoGenDialogueSystemData.IsValid() ? WeakAutoGenDialogueSystemData->GetAutoGenDialogueSequenceConfig()->PreviewDialogueSoundSequence : nullptr;
}

UGenDialogueSequenceConfigBase* FDialogueSequenceExtender::GetAutoGenDialogueSequenceConfig() const
{
	return WeakAutoGenDialogueSystemData.IsValid() ? WeakAutoGenDialogueSystemData->GetAutoGenDialogueSequenceConfig() : nullptr;
}

bool FDialogueSequenceExtender::IsPreviewDialogueSequenceActived()
{
	return WeakSequencer.Pin()->GetFocusedMovieSceneSequence() == GetPreviewDialogueSoundSequence();
}

bool FDialogueSequenceExtender::IsAutoGenDialogueSequenceActived()
{
	return WeakSequencer.Pin()->GetFocusedMovieSceneSequence() == GetAutoGenDialogueSequence();
}

void FDialogueSequenceExtender::OpenPreviewDialogueSoundSequence()
{
	TGuardValue<bool> InnerSequenceSwitchGuard(FDialogueSequenceEditorHelper::bIsInInnerSequenceSwitch, true);
	FAssetEditorManager::Get().OpenEditorForAsset(GetPreviewDialogueSoundSequence());
}

void FDialogueSequenceExtender::OpenAutoGenDialogueSequence()
{
	TGuardValue<bool> InnerSequenceSwitchGuard(FDialogueSequenceEditorHelper::bIsInInnerSequenceSwitch, true);
	FAssetEditorManager::Get().OpenEditorForAsset(GetAutoGenDialogueSequence());
}

void FDialogueSequenceExtender::OnSequenceCreated(TSharedRef<ISequencer> InSequencer)
{
	if (!FDialogueSequenceEditorHelper::bIsInInnerSequenceSwitch)
	{
		ULevelSequence* LevelSequence = Cast<ULevelSequence>(InSequencer->GetFocusedMovieSceneSequence());
		if (!LevelSequence)
		{
			return;
		}

		if (UAutoGenDialogueSystemData* AutoGenDialogueSystemData = LevelSequence->FindMetaData<UAutoGenDialogueSystemData>())
		{
			UAutoGenDialogueSystemData* OldAutoGenDialogueSequence = WeakAutoGenDialogueSystemData.Get();
			WeakAutoGenDialogueSystemData = AutoGenDialogueSystemData;

			if (AutoGenDialogueSystemData->bIsNewCreated)
			{
				AutoGenDialogueSystemData->bIsNewCreated = false;
				FAssetEditorManager::Get().OpenEditorForAsset(GetAutoGenDialogueSequenceConfig());
				// 直接开不行，延迟一帧
				GEditor->GetTimerManager().Get().SetTimerForNextTick([=]()
					{
						OpenPreviewDialogueSoundSequence();
					});
			}
			else if (OldAutoGenDialogueSequence != AutoGenDialogueSystemData)
			{
				GeneratePreviewCharacters();
			}
		}
		else
		{
			WeakAutoGenDialogueSystemData = nullptr;
			DestroyPreviewStandPositionTemplate();
		}
	}
	if (WeakSequencer.IsValid())
	{
		WeakSequencer.Pin()->OnCloseEvent().RemoveAll(this);
	}
	InSequencer->OnCloseEvent().AddRaw(this, &FDialogueSequenceExtender::OnSequencerClosed);
	WeakSequencer = InSequencer;
}

void FDialogueSequenceExtender::OnSequencerClosed(TSharedRef<ISequencer> InSequencer)
{
	if (!FDialogueSequenceEditorHelper::bIsInInnerSequenceSwitch)
	{
		WeakSequencer = nullptr;
		WeakAutoGenDialogueSystemData = nullptr;
		if (PreviewStandPositionTemplate.IsValid())
		{
			DestroyPreviewStandPositionTemplate();
		}
	}
}

UWorld* FDialogueSequenceExtender::GetEditorWorld() const
{
	return GEditor->GetEditorWorldContext().World();
}

void FDialogueSequenceExtender::DestroyPreviewStandPositionTemplate()
{
	for (const TSoftObjectPtr<ACharacter>& CharacterPtr : CachedSourceCharacterInstance)
	{
		if (ACharacter* Character = CharacterPtr.Get())
		{
			Character->SetIsTemporarilyHiddenInEditor(false);
		}
	}
	CachedSourceCharacterInstance.Empty();
	CharacterNameInstanceMap.Empty();
	if (PreviewStandPositionTemplate.IsValid())
	{
		PreviewStandPositionTemplate->Destroy();
	}
	PreviewStandPositionTemplate = nullptr;
}

void FDialogueSequenceExtender::GeneratePreviewCharacters()
{
	FEditorScriptExecutionGuard EditorScriptExecutionGuard;

	DestroyPreviewStandPositionTemplate();

	UGenDialogueSequenceConfigBase* DialogueSequenceConfig = GetAutoGenDialogueSequenceConfig();
	UAutoGenDialogueSystemData* AutoGenDialogueSequence = GetAutoGenDialogueSystemData();

	if (TSubclassOf<ADialogueStandPositionTemplate> StandTemplateType = DialogueSequenceConfig->DialogueStation.DialogueStationTemplate)
	{
		FTransform SpawnTransform = AutoGenDialogueSequence->StandPositionPosition;
		if (AutoGenDialogueSequence->bIsNotSetStandPosition)
		{
			AutoGenDialogueSequence->bIsNotSetStandPosition = false;
			if (const FLevelEditorViewportClient* client = static_cast<const FLevelEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient()))
			{
				FRotator ViewRotation = client->GetViewRotation();
				FRotator Rotatoion = FRotator::ZeroRotator;
				Rotatoion.Yaw = ViewRotation.Yaw;
				SpawnTransform.SetRotation(Rotatoion.Quaternion());
				SpawnTransform.SetLocation(client->GetViewLocation() + ViewRotation.Vector() * 500.f);
				AutoGenDialogueSequence->StandPositionPosition = SpawnTransform;
			}
		}

		const TArray<FDialogueCharacterData>& DialogueCharacterDatas = DialogueSequenceConfig->DialogueStation.DialogueCharacterDatas;
		ADialogueStandPositionTemplate* StandPositionTemplate = GetEditorWorld()->SpawnActorDeferred<ADialogueStandPositionTemplate>(StandTemplateType, SpawnTransform);
		{
			StandPositionTemplate->StandPositions.SetNumZeroed(DialogueCharacterDatas.Num());
			StandPositionTemplate->bSpawnedPreviewCharacter = true;
			StandPositionTemplate->FinishSpawning(AutoGenDialogueSequence->StandPositionPosition, true);
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
				ChildActorTemplate = Character;
				ChildActorComponent->SetChildActorClass(Character->GetClass());
				PreviewCharacter = Cast<ACharacter>(ChildActorComponent->GetChildActor());

				Character->SetIsTemporarilyHiddenInEditor(true);
				CachedSourceCharacterInstance.Add(Character);
			}
			else
			{
				TSubclassOf<ACharacter> PreviewCharacterType = DialogueCharacterData.TypeOverride;
				PreviewCharacterType = PreviewCharacterType ? PreviewCharacterType : StandPosition.PreviewCharacter;
				PreviewCharacterType = PreviewCharacterType ? PreviewCharacterType : StandPositionTemplate->PreviewCharacter;

				if (PreviewCharacterType)
				{
					ChildActorComponent->SetChildActorClass(PreviewCharacterType);
					PreviewCharacter = Cast<ACharacter>(ChildActorComponent->GetChildActor());
				}
			}

			if (PreviewCharacter)
			{
				PreviewCharacter->SetFlags(RF_Transient);
				PreviewCharacter->SetActorLabel(CharacterName.ToString());
				if (PreviewCharacter->Rename(*CharacterName.ToString(), nullptr, REN_Test))
				{
					PreviewCharacter->Rename(*CharacterName.ToString(), nullptr);
				}
				PreviewCharacter->SetActorRelativeLocation(FVector(0.f, 0.f, PreviewCharacter->GetDefaultHalfHeight()));

				CharacterNameInstanceMap.Add(CharacterName, PreviewCharacter);
			}
			ChildActorComponent->SetRelativeTransform(DialogueCharacterData.PositionOverride);
			StandPosition.StandPosition = DialogueCharacterData.PositionOverride;
			SyncSequenceInstanceReference(GetAutoGenDialogueSystemData(), CharacterNameInstanceMap);
		}

		StandPositionTemplate->OnInstanceChanged.BindRaw(this, &FDialogueSequenceExtender::WhenStandTemplateInstanceChanged);
	}
}

void FDialogueSequenceExtender::SyncSequenceInstanceReference(UAutoGenDialogueSystemData* AutoGenDialogueSystemData, const TMap<FName, TSoftObjectPtr<ACharacter>>& CharacterNameInstanceMap)
{
	ULevelSequence* LevelSeqeunce = AutoGenDialogueSystemData->GetOwingLevelSequence();
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

void FDialogueSequenceExtender::WhenStandTemplateInstanceChanged()
{
	ADialogueStandPositionTemplate* StandPositionTemplate = PreviewStandPositionTemplate.Get();
	UGenDialogueSequenceConfigBase* AutoGenDialogueSequenceConfig = GetAutoGenDialogueSequenceConfig();
	StandPositionTemplate->Modify();
	AutoGenDialogueSequenceConfig->Modify();

	AutoGenDialogueSequenceConfig->DialogueStation.SyncInstanceData(StandPositionTemplate);
	GetAutoGenDialogueSystemData()->StandPositionPosition = StandPositionTemplate->GetActorTransform();
}

#undef LOCTEXT_NAMESPACE
