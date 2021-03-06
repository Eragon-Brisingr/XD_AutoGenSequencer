﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoGenEditor/AutoGenSequencerCBExtensions.h"
#include <ContentBrowserModule.h>
#include <ContentBrowserDelegates.h>
#include <Framework/MultiBox/MultiBoxBuilder.h>
#include <EditorStyleSet.h>
#include <Sound/SoundWave.h>
#include <AssetRegistryModule.h>
#include <LevelSequence.h>
#include <ObjectTools.h>
#include <ScopedTransaction.h>

#include "Data/DialogueSentence.h"
#include "Utils/AutoGenDialogueSettings.h"
#include "Factory/AutoGenDialogueSequenceFactory.h"
#include "AutoGenEditor/GenDialogueSequenceConfigBase.h"

#define LOCTEXT_NAMESPACE "FAutoGenSequencer_EditorModule"

FDelegateHandle FAutoGenSequencerContentBrowserExtensions::ContentBrowserExtenderDelegateHandle;

void FAutoGenSequencerContentBrowserExtensions::RegisterExtender()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	CBMenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&FAutoGenSequencerContentBrowserExtensions::OnExtendContentBrowserAssetSelectionMenu));
	ContentBrowserExtenderDelegateHandle = CBMenuExtenderDelegates.Last().GetHandle();
}

void FAutoGenSequencerContentBrowserExtensions::UnregisterExtender()
{
	if (FModuleManager::Get().IsModuleLoaded("ContentBrowser"))
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
		CBMenuExtenderDelegates.RemoveAll([](const FContentBrowserMenuExtender_SelectedAssets& Delegate) { return Delegate.GetHandle() == ContentBrowserExtenderDelegateHandle; });
	}
}

TSharedRef<FExtender> FAutoGenSequencerContentBrowserExtensions::OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender(new FExtender());

	// 音频转换成对话资源
	{
		TArray<FAssetData> SelectedSoundWaves;
		for (const FAssetData& Asset : SelectedAssets)
		{
			if (Asset.AssetClass == USoundWave::StaticClass()->GetFName())
			{
				SelectedSoundWaves.Add(Asset);
			}
		}
		if (SelectedSoundWaves.Num() > 0)
		{
			Extender->AddMenuExtension(
				TEXT("GetAssetActions"),
				EExtensionHook::After,
				nullptr,
				FMenuExtensionDelegate::CreateLambda([=](FMenuBuilder& MenuBuilder)
					{
						MenuBuilder.AddMenuEntry(
							LOCTEXT("CreateDialogueSentence_Menu", "生成对白语句"),
							LOCTEXT("CreateDialogueSentence_Tooltip", "生成对白语句"),
							FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([=]()
								{
									for (const FAssetData& SoundWaveAsset : SelectedSoundWaves)
									{
										USoundWave* SoundWave = CastChecked<USoundWave>(SoundWaveAsset.GetAsset());

										FString SequenceName = FString::Printf(TEXT("%s_DialogueSentence"), *SoundWaveAsset.AssetName.ToString());
										FString DialogueSentencePath = FString::Printf(TEXT("%s_DialogueSentence"), *SoundWaveAsset.PackageName.ToString());

										if (UPackage* Package = LoadObject<UPackage>(nullptr, *DialogueSentencePath))
										{
											ObjectTools::ForceDeleteObjects({ Package }, false);
										}

										UPackage* DialogueSentencePackage = CreatePackage(nullptr, *DialogueSentencePath);
										TSubclassOf<UDialogueSentence> DialogueSentenceType = UAutoGenDialogueSettings::GetDialogueSentenceType();
										UDialogueSentence* DialogueSentence = NewObject<UDialogueSentence>(DialogueSentencePackage, DialogueSentenceType, *SequenceName, RF_Public | RF_Standalone);
										DialogueSentence->SentenceWave = SoundWave;
										DialogueSentence->SubTitle = FText::FromString(SoundWave->SpokenText);
										DialogueSentence->WhenGenFromSoundWave(SoundWave);

										FAssetRegistryModule::AssetCreated(DialogueSentence);
										DialogueSentence->MarkPackageDirty();
									}
								})));
					}));
		}
	}

	// LevelSequence添加自动生成对话功能
	{
		TArray<FAssetData> SelectedLevelSequences;
		for (const FAssetData& Asset : SelectedAssets)
		{
			if (Asset.AssetClass == ULevelSequence::StaticClass()->GetFName())
			{
				SelectedLevelSequences.Add(Asset);
			}
		}
		if (SelectedLevelSequences.Num() > 0)
		{
			Extender->AddMenuExtension(
				TEXT("CommonAssetActions"),
				EExtensionHook::After,
				nullptr,
				FMenuExtensionDelegate::CreateLambda([=](FMenuBuilder& MenuBuilder)
					{
						MenuBuilder.AddMenuEntry(
							LOCTEXT("AddAutoGenDialogueSystem_Menu", "添加自动生成对白系统"),
							LOCTEXT("AddAutoGenDialogueSystem_Tooltip", "添加自动生成对白系统，具备自动生成对话序列的功能"),
							FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([=]()
								{
									UClass* ChoseClass = nullptr;
									const bool bIsPressedOk = UAutoGenDialogueSequenceFactory::ShowPickConfigClassViewer(ChoseClass);
									if (bIsPressedOk && ChoseClass)
									{
										for (const FAssetData& LevelSequenceAsset : SelectedLevelSequences)
										{
											ULevelSequence* LevelSequence = CastChecked<ULevelSequence>(LevelSequenceAsset.GetAsset());
											UGenDialogueSequenceConfigContainer* GenDialogueSequenceConfigContainer = LevelSequence->FindMetaData<UGenDialogueSequenceConfigContainer>();
											if (GenDialogueSequenceConfigContainer && !GenDialogueSequenceConfigContainer->GenDialogueSequenceConfig->IsA(ChoseClass))
											{
												LevelSequence->RemoveMetaData<UGenDialogueSequenceConfigContainer>();
												GenDialogueSequenceConfigContainer = nullptr;
											}
											if (GenDialogueSequenceConfigContainer == nullptr)
											{
												UAutoGenDialogueSequenceFactory::AddGenDialogueSequenceConfig(LevelSequence, ChoseClass);
											}
											LevelSequence->MarkPackageDirty();
										}
									}
								})));

						MenuBuilder.AddMenuEntry(
							LOCTEXT("RemoveAutoGenDialogueSystem_Menu", "移除自动生成对白系统"),
							LOCTEXT("RemoveAutoGenDialogueSystem_Tooltip", "移除自动生成对白系统，回退为原始的LevelSequence，不具备再次生成对话序列的功能"),
							FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([=]()
								{
									for (const FAssetData& LevelSequenceAsset : SelectedLevelSequences)
									{
										ULevelSequence* LevelSequence = CastChecked<ULevelSequence>(LevelSequenceAsset.GetAsset());
										LevelSequence->RemoveMetaData<UGenDialogueSequenceConfigContainer>();
										LevelSequence->MarkPackageDirty();
									}
								})));
					}));
		}
	}

	return Extender;
}

#undef LOCTEXT_NAMESPACE
