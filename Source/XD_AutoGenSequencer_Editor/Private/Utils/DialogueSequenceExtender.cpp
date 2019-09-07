// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueSequenceExtender.h"

#include "GenDialogueSequenceConfigBase.h"
#include "AutoGenDialogueSequence.h"
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
		"Level Sequence Separator",
		EExtensionHook::After,
		nullptr,
		FToolBarExtensionDelegate::CreateLambda([=](FToolBarBuilder& ToolBarBuilder)
			{
				ToolBarBuilder.AddToolBarButton(FUIAction(FExecuteAction::CreateLambda([=]()
						{
							if (UPreviewDialogueSoundSequence* PreviewDialogueSoundSequence = GetPreviewDialogueSoundSequence())
							{
								FAssetEditorManager::Get().OpenEditorForAsset(GetAutoGenDialogueSequenceConfig());
							}
						}),
					FCanExecuteAction::CreateLambda([=]()
						{
							return IsPreviewDialogueSequenceActived();
						}),
					FIsActionChecked(),
					FIsActionButtonVisible::CreateLambda([=]()
						{
							return WeakAutoGenDialogueSequence.IsValid();
						})), NAME_None,
					LOCTEXT("打开对白配置", "打开对白配置"),
					LOCTEXT("打开对白配置", "打开对白配置"),
					FAutoGenDialogueEditorStyle::Get().GetConfigIcon());
				ToolBarBuilder.AddToolBarButton(FUIAction(FExecuteAction::CreateLambda([=]()
						{
							if (UPreviewDialogueSoundSequence* PreviewDialogueSoundSequence = GetPreviewDialogueSoundSequence())
							{
								TGuardValue<bool> InnerSequenceSwitchGuard(FDialogueSequenceEditorHelper::bIsInInnerSequenceSwitch, true);
								FAssetEditorManager::Get().OpenEditorForAsset(PreviewDialogueSoundSequence);
							}
						}),
					FCanExecuteAction::CreateLambda([=]()
						{
							return IsAutoGenDialogueSequenceActived();
						}),
					FIsActionChecked(),
					FIsActionButtonVisible::CreateLambda([=]()
						{
							return WeakAutoGenDialogueSequence.IsValid();
						})), NAME_None,
					LOCTEXT("打开预览序列", "打开预览序列"),
					LOCTEXT("打开预览序列", "打开预览序列"),
					FAutoGenDialogueEditorStyle::Get().GetPreviewIcon());
				ToolBarBuilder.AddToolBarButton(FUIAction(FExecuteAction::CreateLambda([=]()
					{
						if (UAutoGenDialogueSequence* AutoGenDialogueSequence = GetAutoGenDialogueSequence())
						{
							TGuardValue<bool> InnerSequenceSwitchGuard(FDialogueSequenceEditorHelper::bIsInInnerSequenceSwitch, true);
							FAssetEditorManager::Get().OpenEditorForAsset(AutoGenDialogueSequence);
						}
					}),
					FCanExecuteAction::CreateLambda([=]()
						{
							return IsPreviewDialogueSequenceActived();
						}),
					FIsActionChecked(),
					FIsActionButtonVisible::CreateLambda([=]()
						{
							return WeakAutoGenDialogueSequence.IsValid();
						})), NAME_None,
					LOCTEXT("打开对白序列", "打开对白序列"),
					LOCTEXT("打开对白序列", "打开对白序列"),
					FAutoGenDialogueEditorStyle::Get().GetDialogueIcon());
				ToolBarBuilder.AddToolBarButton(FUIAction(FExecuteAction::CreateLambda([=]()
						{
							if (IsPreviewDialogueSequenceActived())
							{
								UPreviewDialogueSoundSequence* PreviewDialogueSoundSequence = GetPreviewDialogueSoundSequence();
								UGenDialogueSequenceConfigBase* Config = PreviewDialogueSoundSequence->GetDialogueConfig();
								if (!Config->IsConfigValid())
								{
									FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("预览导轨生成报错", "配置中存在问题，无法生成"));
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
								UAutoGenDialogueSequence* AutoGenDialogueSequence = GetAutoGenDialogueSequence();
								if (!PreviewStandPositionTemplate.IsValid())
								{
									GeneratePreviewCharacters();
								}

								UGenDialogueSequenceConfigBase* GenDialogueSequenceConfig = AutoGenDialogueSequence->GetAutoGenDialogueSequenceConfig();
								GenDialogueSequenceConfig->Generate(WeakSequencer.Pin().ToSharedRef(), GetEditorWorld(), CharacterNameInstanceMap, *AutoGenDialogueSequence);

								TGuardValue<bool> InnerSequenceSwitchGuard(FDialogueSequenceEditorHelper::bIsInInnerSequenceSwitch, true);
								//不知道怎么直接刷新，临时切换下来刷新 ISequencer::NotifyMovieSceneDataChanged
								FAssetEditorManager::Get().OpenEditorForAsset(GetPreviewDialogueSoundSequence());
								FAssetEditorManager::Get().OpenEditorForAsset(AutoGenDialogueSequence);
							}
						}),
					FCanExecuteAction::CreateLambda([]()
						{
							return true;
						}),
					FIsActionChecked(),
					FIsActionButtonVisible::CreateLambda([=]()
						{
							return WeakAutoGenDialogueSequence.IsValid();
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
							return WeakAutoGenDialogueSequence.IsValid();
						}), EUIActionRepeatMode::RepeatEnabled), NAME_None,
					LOCTEXT("刷新模板显示", "刷新模板显示"),
					LOCTEXT("刷新模板显示", "刷新模板显示"),
					FAutoGenDialogueEditorStyle::Get().GetStandpositionIcon());

				ToolBarBuilder.AddSeparator("Auto Gen Dialogue");
			}));

	SequencerModule.GetToolBarExtensibilityManager()->AddExtender(SequencerToolbarExtender);
}

void FDialogueSequenceExtender::Unregister(ISequencerModule& SequencerModule)
{
	SequencerModule.UnregisterSequenceEditor(SequencerCreatedHandle);
	SequencerModule.GetToolBarExtensibilityManager()->RemoveExtender(SequencerToolbarExtender);
}

UAutoGenDialogueSequence* FDialogueSequenceExtender::GetAutoGenDialogueSequence() const
{
	return WeakAutoGenDialogueSequence.Get();
}

UPreviewDialogueSoundSequence* FDialogueSequenceExtender::GetPreviewDialogueSoundSequence() const
{
	return WeakAutoGenDialogueSequence.IsValid() ? WeakAutoGenDialogueSequence->GetAutoGenDialogueSequenceConfig()->PreviewDialogueSoundSequence : nullptr;
}

UGenDialogueSequenceConfigBase* FDialogueSequenceExtender::GetAutoGenDialogueSequenceConfig() const
{
	return WeakAutoGenDialogueSequence.IsValid() ? WeakAutoGenDialogueSequence->GetAutoGenDialogueSequenceConfig() : nullptr;
}

bool FDialogueSequenceExtender::IsPreviewDialogueSequenceActived()
{
	if (WeakSequencer.IsValid())
	{
		return Cast<UPreviewDialogueSoundSequence>(WeakSequencer.Pin()->GetFocusedMovieSceneSequence()) ? true : false;
	}
	return false;
}

bool FDialogueSequenceExtender::IsAutoGenDialogueSequenceActived()
{
	if (WeakSequencer.IsValid())
	{
		return Cast<UAutoGenDialogueSequence>(WeakSequencer.Pin()->GetFocusedMovieSceneSequence()) ? true : false;
	}
	return false;
}

void FDialogueSequenceExtender::OnSequenceCreated(TSharedRef<ISequencer> InSequencer)
{
	if (!FDialogueSequenceEditorHelper::bIsInInnerSequenceSwitch)
	{
		if (UAutoGenDialogueSequence* AutoGenDialogueSequence = Cast<UAutoGenDialogueSequence>(InSequencer->GetFocusedMovieSceneSequence()))
		{
			UAutoGenDialogueSequence* OldAutoGenDialogueSequence = WeakAutoGenDialogueSequence.Get();
			WeakAutoGenDialogueSequence = AutoGenDialogueSequence;

			if (AutoGenDialogueSequence->bIsNewCreated)
			{
				AutoGenDialogueSequence->bIsNewCreated = false;
				TGuardValue<bool> InnerSequenceSwitchGuard(FDialogueSequenceEditorHelper::bIsInInnerSequenceSwitch, true);
				FAssetEditorManager::Get().OpenEditorForAsset(GetPreviewDialogueSoundSequence());
				FAssetEditorManager::Get().OpenEditorForAsset(GetAutoGenDialogueSequenceConfig());
			}
			else if (OldAutoGenDialogueSequence != AutoGenDialogueSequence)
			{
				GeneratePreviewCharacters();
			}
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
		WeakAutoGenDialogueSequence = nullptr;
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
	DestroyPreviewStandPositionTemplate();

	UGenDialogueSequenceConfigBase* DialogueSequenceConfig = GetAutoGenDialogueSequenceConfig();
	UAutoGenDialogueSequence* AutoGenDialogueSequence = GetAutoGenDialogueSequence();

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
			SyncSequenceInstanceReference(GetAutoGenDialogueSequence(), CharacterNameInstanceMap);
		}

		StandPositionTemplate->OnInstanceChanged.BindRaw(this, &FDialogueSequenceExtender::WhenStandTemplateInstanceChanged);
	}
}

void FDialogueSequenceExtender::SyncSequenceInstanceReference(UAutoGenDialogueSequence* AutoGenDialogueSequence, const TMap<FName, TSoftObjectPtr<ACharacter>>& CharacterNameInstanceMap)
{
	UMovieScene* MovieScene = AutoGenDialogueSequence->GetMovieScene();
	for (const TPair<FName, TSoftObjectPtr<ACharacter>>& Pair : CharacterNameInstanceMap)
	{
		if (FMovieScenePossessable* MovieScenePossessable = MovieScene->FindPossessable([=](const FMovieScenePossessable& E) {return E.GetName() == Pair.Key.ToString(); }))
		{
			ACharacter* SpeakerInstance = Pair.Value.Get();
			check(SpeakerInstance);
			FGuid BindingGuid = MovieScenePossessable->GetGuid();
			AutoGenDialogueSequence->UnbindPossessableObjects(BindingGuid);
			AutoGenDialogueSequence->BindPossessableObject(MovieScenePossessable->GetGuid(), *SpeakerInstance, SpeakerInstance);
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
	GetAutoGenDialogueSequence()->StandPositionPosition = StandPositionTemplate->GetActorTransform();
}

#undef LOCTEXT_NAMESPACE
