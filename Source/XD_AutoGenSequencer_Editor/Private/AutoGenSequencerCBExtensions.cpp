// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoGenSequencerCBExtensions.h"

#include "ContentBrowserModule.h"
#include "ContentBrowserDelegates.h"
#include "LevelSequence.h"
#include "MultiBoxBuilder.h"
#include "EditorStyleSet.h"
#include "GameFramework/Actor.h"
#include "MovieSceneTransformTrack.h"
#include "EngineUtils.h"
#include "Editor/EditorEngine.h"
#include "Editor.h"
#include "Engine/Selection.h"
#include "MovieScene3DTransformTrack.h"
#include "MovieScene3DTransformSection.h"
#include "MovieSceneFloatChannel.h"
#include "MovieSceneChannelProxy.h"
#include "MovieSceneSkeletalAnimationTrack.h"
#include "Animation/AnimSequence.h"
#include "MovieSceneCameraCutTrack.h"
#include "MovieSceneFolder.h"

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
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	CBMenuExtenderDelegates.RemoveAll([](const FContentBrowserMenuExtender_SelectedAssets& Delegate) { return Delegate.GetHandle() == ContentBrowserExtenderDelegateHandle; });
}

TSharedRef<FExtender> FAutoGenSequencerContentBrowserExtensions::OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender(new FExtender());

	// Run thru the assets to determine if any meet our criteria
	bool bAnySequencer = false;
	for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
	{
		const FAssetData& Asset = *AssetIt;
		bAnySequencer = bAnySequencer || (Asset.AssetClass == ULevelSequence::StaticClass()->GetFName());
	}

	if (bAnySequencer)
	{
		// Add the sprite actions sub-menu extender
		Extender->AddMenuExtension(
			"CommonAssetActions",
			EExtensionHook::After,
			nullptr,
			FMenuExtensionDelegate::CreateStatic(&FAutoGenSequencerContentBrowserExtensions::CreateAutoGenSequencerSubMenu, SelectedAssets));
	}

	return Extender;
}

void FAutoGenSequencerContentBrowserExtensions::CreateAutoGenSequencerSubMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
{
	MenuBuilder.AddMenuEntry(
		LOCTEXT("AutoGenSequencerMenuLabel", "自动生成轨道"),
		LOCTEXT("AutoGenSequencerMenuToolTip", "自动生成轨道"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([=]()
			{
				for (const FAssetData& AssetData : SelectedAssets)
				{
					ULevelSequence* LevelSequence = CastChecked<ULevelSequence>(AssetData.GetAsset());
					UMovieScene* MovieScene = LevelSequence->MovieScene;

					{
						UMovieSceneCameraCutTrack* Track = CastChecked<UMovieSceneCameraCutTrack>(MovieScene->AddCameraCutTrack(UMovieSceneCameraCutTrack::StaticClass()));
						// TODO:添加摄像机和绑定
						//Track->AddNewCameraCut();
					}

					UMovieSceneFolder* Folder = NewObject<UMovieSceneFolder>(MovieScene, NAME_None, RF_Transactional);
					Folder->SetFolderName(TEXT("自动生成"));
					MovieScene->GetRootFolders().Add(Folder);

					for (FSelectionIterator Iter(*GEditor->GetSelectedActors()); Iter; ++Iter)
					{
						AActor* SelectedActor = CastChecked<AActor>(*Iter);

						struct FLevelSequenceFunctionHelper : public ULevelSequence
						{
							static FGuid ExecuteCreatePossessable(ULevelSequence* LevelSequence, UObject* ObjectToPossess)
							{
								return static_cast<FLevelSequenceFunctionHelper*>(LevelSequence)->CreatePossessable(ObjectToPossess);
							}
						};

						FGuid BindingGuid = FLevelSequenceFunctionHelper::ExecuteCreatePossessable(LevelSequence, SelectedActor);

						Folder->AddChildObjectBinding(BindingGuid);

						{
							UMovieScene3DTransformTrack* Track = MovieScene->AddTrack<UMovieScene3DTransformTrack>(BindingGuid);
							UMovieScene3DTransformSection* Section = Cast<UMovieScene3DTransformSection>(Track->CreateNewSection());
							Track->AddSection(*Section);
							TArrayView<FMovieSceneFloatChannel*> FloatChannels = Section->GetChannelProxy().GetChannels<FMovieSceneFloatChannel>();
							FloatChannels[0]->AddConstantKey(0, 0.f);
							FloatChannels[1]->AddConstantKey(0, 0.f);
							FloatChannels[2]->AddConstantKey(0, 0.f);
						}

						{
							UMovieSceneSkeletalAnimationTrack* Track = MovieScene->AddTrack<UMovieSceneSkeletalAnimationTrack>(BindingGuid);
							Track->AddNewAnimation(0, NewObject<UAnimSequence>());
						}
					}
				}
			}))
	);
}

#undef LOCTEXT_NAMESPACE
