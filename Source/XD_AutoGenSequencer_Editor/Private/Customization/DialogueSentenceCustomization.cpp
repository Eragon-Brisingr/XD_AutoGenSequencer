// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueSentenceCustomization.h"
#include "AutoGenDialogueSequenceConfig.h"
#include "DetailWidgetRow.h"
#include "SBoxPanel.h"
#include "GameFramework/Character.h"
#include "DialogueInterface.h"
#include "IDetailChildrenBuilder.h"

namespace CustomizationUtils
{
	UObject* GetOuter(const TSharedRef<IPropertyHandle>& PropertyHandle)
	{
		TArray<UObject*> Outers;
		PropertyHandle->GetOuterObjects(Outers);
		return Outers[0];
	}

	template<typename ValueType>
	ValueType GetValue(const TSharedPtr<IPropertyHandle>& PropertyHandle)
	{
		if (UObject* Outer = GetOuter(PropertyHandle.ToSharedRef()))
		{
			if (ValueType* Res = reinterpret_cast<ValueType*>(PropertyHandle->GetValueBaseAddress(reinterpret_cast<uint8*>(Outer))))
			{
				return *Res;
			}
		}
		return {};
	}

	template<typename Type>
	void SetValue(const TSharedPtr<IPropertyHandle>& PropertyHandle, const Type& Value, bool NotifyChange = true)
	{
		if (NotifyChange)
		{
			PropertyHandle->NotifyPreChange();
		}
		if (Type* Target = reinterpret_cast<Type*>(PropertyHandle->GetValueBaseAddress(reinterpret_cast<uint8*>(GetOuter(PropertyHandle.ToSharedRef())))))
		{
			*Target = Value;
			if (NotifyChange)
			{
				PropertyHandle->NotifyPostChange(EPropertyValueSetFlags::DefaultFlags);
			}
		}
	}
}

void FDialogueStationInstanceOverride_Customization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<class IPropertyHandle> InstanceOverridePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDialogueStationInstanceOverride, InstanceOverride));

	InstanceOverridePropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=]()
		{
			FDialogueStationInstanceOverride DialogueStationInstanceOverride = CustomizationUtils::GetValue<FDialogueStationInstanceOverride>(StructPropertyHandle);
			if (ACharacter* Character = DialogueStationInstanceOverride.InstanceOverride.Get())
			{
				DialogueStationInstanceOverride.TypeOverride = Character->GetClass();
				if (IDialogueInterface* DialogueInterface = Cast<IDialogueInterface>(Character))
				{
					DialogueStationInstanceOverride.DialogueVoiceOverride = DialogueInterface->GetDialogueVoice();
					DialogueStationInstanceOverride.NameOverride = DialogueInterface->GetDialogueCharacterName();
				}
			}
			CustomizationUtils::SetValue(StructPropertyHandle, DialogueStationInstanceOverride);
		}));

	HeaderRow.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		]
	.ValueContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			[
				InstanceOverridePropertyHandle->CreatePropertyValueWidget()
			]
		];
}

void FDialogueStationInstanceOverride_Customization::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TArray<FName> ExcludePropertyNames = {GET_MEMBER_NAME_CHECKED(FDialogueStationInstanceOverride, InstanceOverride) };

	uint32 ChildNum;
	StructPropertyHandle->GetNumChildren(ChildNum);
	for (uint32 ChildIndex = 0; ChildIndex < ChildNum; ++ChildIndex)
	{
		const TSharedRef<IPropertyHandle> ChildHandle = StructPropertyHandle->GetChildHandle(ChildIndex).ToSharedRef();

		if (!ExcludePropertyNames.Contains(*ChildHandle->GetProperty()->GetNameCPP()))
		{
			StructBuilder.AddProperty(ChildHandle);
		}
	}
}

void FDialogueSentenceEditData_Customization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<class IPropertyHandle> DialogueWavePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDialogueSentenceEditData, DialogueWave));
	HeaderRow.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		]
	.ValueContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			[
				DialogueWavePropertyHandle->CreatePropertyValueWidget()
			]
		];
}

void FDialogueSentenceEditData_Customization::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{

}
