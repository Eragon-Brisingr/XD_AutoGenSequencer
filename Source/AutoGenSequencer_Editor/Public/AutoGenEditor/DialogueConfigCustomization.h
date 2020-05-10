// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

/**
 * 
 */
class AUTOGENSEQUENCER_EDITOR_API FDialogueCharacterData_Customization : public IPropertyTypeCustomization
{
public:
	void CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	void CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
	static FName PreNameOverride;
};

class AUTOGENSEQUENCER_EDITOR_API FDialogueSentenceEditData_Customization : public IPropertyTypeCustomization
{
public:
	void CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	void CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
	static FSlateBrush ErrorBrush;
	static FColor ValidColor;
	static FColor ErrorColor;
	static FColor NormalColor;
};

class AUTOGENSEQUENCER_EDITOR_API FDialogueCharacterName_Customization : public IPropertyTypeCustomization
{
public:
	void CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	void CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override{}

	static TSharedRef<SWidget> CreateValueWidget(TSharedRef<class IPropertyHandle> StructPropertyHandle, TArray<TSharedPtr<FName>>& DialogueNameList);
};
