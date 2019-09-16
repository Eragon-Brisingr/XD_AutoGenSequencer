// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoGenSequencerCBExtensions.h"

#include "ContentBrowserModule.h"
#include "ContentBrowserDelegates.h"
#include "MultiBoxBuilder.h"
#include "EditorStyleSet.h"
#include "Sound/SoundWave.h"
#include "DialogueSentence.h"
#include "AssetRegistryModule.h"
#include "AutoGenDialogueSettings.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

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

	bool bAnySoundWave = false;

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
			"GetAssetActions",
			EExtensionHook::After,
			nullptr,
			FMenuExtensionDelegate::CreateLambda([=](FMenuBuilder& MenuBuilder)
				{
					MenuBuilder.AddMenuEntry(
						LOCTEXT("CreateDialogueSentence_Menu", "生成对白语句"),
						LOCTEXT("CreateDialogueSentence_Tooltip",
							"生成对白语句"),
						FSlateIcon(), FUIAction(FExecuteAction::CreateLambda([=]()
							{
								for (const FAssetData& SoundWaveAsset : SelectedSoundWaves)
								{
									USoundWave* SoundWave = CastChecked<USoundWave>(SoundWaveAsset.GetAsset());

									FString SequenceName = FString::Printf(TEXT("%s_DialogueSentence"), *SoundWaveAsset.AssetName.ToString());
									FString DialogueSentencePath = FString::Printf(TEXT("%s_DialogueSentence"), *SoundWaveAsset.PackageName.ToString());

									// TODO：避免已存在的资源
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

	return Extender;
}

#undef LOCTEXT_NAMESPACE
