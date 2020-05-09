// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FExtender;
struct FAssetData;
class FMenuBuilder;

/**
 * 
 */
struct FAutoGenSequencerContentBrowserExtensions
{
	static FDelegateHandle ContentBrowserExtenderDelegateHandle;

	static void RegisterExtender();

	static void UnregisterExtender();

	static TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets);

	static void CreateAutoGenSequencerSubMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);
};
