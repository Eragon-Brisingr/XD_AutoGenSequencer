// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <UObject/NoExportTypes.h>
#include <Factories/Factory.h>
#include <AssetTypeActions_Base.h>
#include "AutoGenDialogueCharacterSettings.generated.h"

/**
 * 
 */
UCLASS()
class XD_AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueCharacterSettings : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "说话动画插槽"))
	FName TalkAnimSlotName = TEXT("DefaultSlot");

	// 对应的角色定义中必须存在该命名的Character软引用类型变量
	UPROPERTY(EditAnywhere, meta = (DisplayName = "注视目标属性名"))
	FName LookAtTargetPropertyName = TEXT("CineLookAtTarget");
};

UCLASS()
class XD_AUTOGENSEQUENCER_EDITOR_API UAutoGenDialogueCharacterSettingsFactory : public UFactory
{
	GENERATED_BODY()
public:
	UAutoGenDialogueCharacterSettingsFactory();

	UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;

	FText GetDisplayName() const override;
};

class FAssetTypeActions_AutoGenDialogueCharacterSettings : public FAssetTypeActions_Base
{
	using Super = FAssetTypeActions_Base;

	// Inherited via FAssetTypeActions_Base
	FText GetName() const override;
	UClass* GetSupportedClass() const override;
	FColor GetTypeColor() const override;
	uint32 GetCategories() override;
};

