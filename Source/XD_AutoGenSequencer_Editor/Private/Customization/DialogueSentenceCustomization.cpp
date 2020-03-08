// Fill out your copyright notice in the Description page of Project Settings.


#include "Customization/DialogueSentenceCustomization.h"
#include <DetailWidgetRow.h>
#include <Widgets/SBoxPanel.h>
#include <GameFramework/Character.h>
#include <IDetailChildrenBuilder.h>
#include <Editor.h>
#include <Editor/EditorEngine.h>
#include <TimerManager.h>
#include <Widgets/Input/SButton.h>
#include <EditorStyleSet.h>
#include <Widgets/Images/SImage.h>
#include <Misc/MessageDialog.h>
#include <SNameComboBox.h>
#include <IPropertyTypeCustomization.h>
#include <IPropertyUtilities.h>

#include "Datas/GenDialogueSequenceConfigBase.h"
#include "Datas/AutoGenDialogueSequenceConfig.h"
#include "Interface/XD_AutoGenDialogueInterface.h"
#include "Utils/AutoGenSequence_Log.h"

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

	static uint32 GetNumChildren(const TSharedRef<IPropertyHandle>& PropertyHandle)
	{
		uint32 NumChildren;
		PropertyHandle->GetNumChildren(NumChildren);
		return NumChildren;
	}

	static void StructBuilderDrawPropertys(class IDetailChildrenBuilder& StructBuilder, const TSharedRef<IPropertyHandle>& PropertyHandle, const TArray<FName>& ExcludePropertyNames = {})
	{
		for (uint32 ChildIndex = 0; ChildIndex < GetNumChildren(PropertyHandle); ++ChildIndex)
		{
			const TSharedRef<IPropertyHandle> ChildHandle = PropertyHandle->GetChildHandle(ChildIndex).ToSharedRef();

			if (!ExcludePropertyNames.Contains(*ChildHandle->GetProperty()->GetNameCPP()))
			{
				StructBuilder.AddProperty(ChildHandle);
			}
		}
	}
}

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

void FDialogueCharacterData_Customization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> InstanceOverridePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDialogueCharacterData, InstanceOverride));

	InstanceOverridePropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=]()
		{
			FDialogueCharacterData DialogueStationInstanceOverride = CustomizationUtils::GetValue<FDialogueCharacterData>(StructPropertyHandle);
			if (ACharacter* Character = DialogueStationInstanceOverride.InstanceOverride.Get())
			{
				DialogueStationInstanceOverride.TypeOverride = Character->GetClass();
				if (Character->Implements<UXD_AutoGenDialogueInterface>())
				{
					DialogueStationInstanceOverride.NameOverride = IXD_AutoGenDialogueInterface::GetDialogueCharacterName(Character);
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

void FDialogueCharacterData_Customization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	CustomizationUtils::StructBuilderDrawPropertys(StructBuilder, StructPropertyHandle, { GET_MEMBER_NAME_CHECKED(FDialogueCharacterData, InstanceOverride) });

	if (UGenDialogueSequenceConfigBase* Config = Cast<UGenDialogueSequenceConfigBase>(CustomizationUtils::GetOuter(StructPropertyHandle)))
	{
		TSharedPtr<IPropertyHandle> NameOverridePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDialogueCharacterData, NameOverride));
		NameOverridePropertyHandle->SetOnPropertyValuePreChange(FSimpleDelegate::CreateLambda([=]()
			{
				NameOverridePropertyHandle->GetValue(PreNameOverride);
			}));
		NameOverridePropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=]()
			{
				FName NewNameOverride;
				NameOverridePropertyHandle->GetValue(NewNameOverride);
				
				int32 SameNameNum = 0;
				for (FDialogueCharacterData& DialogueStationInstanceOverride : Config->DialogueCharacterDatas)
				{
					if (DialogueStationInstanceOverride.NameOverride == NewNameOverride)
					{
						SameNameNum += 1;
					}
				}

				if (NewNameOverride == NAME_None)
				{
					FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("重名命对白角色名报错_命名为空", "角色名不得为空"));
					CustomizationUtils::SetValue(NameOverridePropertyHandle, PreNameOverride, false);
					return;
				}
				else if (SameNameNum > 1)
				{
					FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("重名命对白角色名报错_重名", "角色名不得与其他人重名"));
					CustomizationUtils::SetValue(NameOverridePropertyHandle, PreNameOverride, false);
					return;
				}
				else
				{
					Config->WhenCharacterNameChanged(PreNameOverride, NewNameOverride);
				}
			}));
	}
}

FName FDialogueCharacterData_Customization::PreNameOverride;

FSlateBrush FDialogueSentenceEditData_Customization::ErrorBrush;
FColor FDialogueSentenceEditData_Customization::ValidColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.f).ToFColor(false);
FColor FDialogueSentenceEditData_Customization::ErrorColor = FLinearColor(0.8f, 0.2f, 0.2f, 1.f).ToFColor(false);
FColor FDialogueSentenceEditData_Customization::NormalColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.f).ToFColor(false);

void FDialogueSentenceEditData_Customization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	UAutoGenDialogueSequenceConfig* Config = Cast<UAutoGenDialogueSequenceConfig>(CustomizationUtils::GetOuter(StructPropertyHandle));
	if (!Config)
	{
		return;
	}

	TSharedPtr<IPropertyUtilities> PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();
	TSharedPtr<IPropertyHandle> DialogueSentencePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDialogueSentenceEditData, DialogueSentence));
	TSharedPtr<IPropertyHandle> SpeakerNamePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDialogueSentenceEditData, SpeakerName));
	TSharedPtr<IPropertyHandle> ToAllTargetsPropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDialogueSentenceEditData, bToAllTargets));
	TSharedPtr<IPropertyHandle> TargetNamesPropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDialogueSentenceEditData, TargetNames));

	TSharedRef<SHorizontalBox> TargetsWidget = SNew(SHorizontalBox)
		.Visibility_Lambda([=]() 
			{
				bool bToAllTargets;
				ToAllTargetsPropertyHandle->GetValue(bToAllTargets);
				return bToAllTargets == false ? EVisibility::Visible : EVisibility::Collapsed;
			});
	{
		auto CreateChilds = [=]()
			{
				TSharedPtr<IPropertyHandleArray> TargetNamesArrayPropertyHandle = TargetNamesPropertyHandle->AsArray();
				TargetsWidget->ClearChildren();
				uint32 Num;
				TargetNamesArrayPropertyHandle->GetNumElements(Num);
				for (uint32 Idx = 0; Idx < Num; ++Idx)
				{
					TSharedRef<IPropertyHandle> Element = TargetNamesArrayPropertyHandle->GetElement(Idx);
					TargetsWidget->AddSlot()
						.AutoWidth()
						[
							FDialogueCharacterName_Customization::CreateValueWidget(Element, Config->GetDialogueNameList())
						];
					TargetsWidget->AddSlot()
						.AutoWidth()
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.ButtonStyle(FEditorStyle::Get(), "LayerBrowserButton")
							.ContentPadding(0)
							.OnClicked_Lambda([=]()
								{
									TargetNamesArrayPropertyHandle->DeleteItem(Idx);
									return FReply::Handled();
								})
							.ToolTipText(LOCTEXT("移除对白目标按钮", "移除对白目标"))
							[
								SNew(SImage)
								.Image(FEditorStyle::GetBrush(TEXT("LayerBrowser.Actor.RemoveFromLayer")))
							]
						];
				}
			};
		CreateChilds();
		TargetNamesPropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=]()
			{
				CreateChilds();
				PropertyUtilities->ForceRefresh();
			}));
	}

	HeaderRow.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		]
	.ValueContent()
		.HAlign(HAlign_Fill)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 0.f)
			[
				SNew(SBorder)
				.BorderImage(&ErrorBrush)
				.BorderBackgroundColor_Lambda([=]()
					{
						FDialogueSentenceEditData Data = CustomizationUtils::GetValue<FDialogueSentenceEditData>(StructPropertyHandle);
						TArray<FText> ErrorText;
						return Config->IsDialogueSentenceEditDataValid(Data, Config->GetCharacterNames(), ErrorText) ? ValidColor : ErrorColor;
					})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(2.f, 0.f)
					.AutoWidth()
					[
						SpeakerNamePropertyHandle->CreatePropertyNameWidget()
					]
					+ SHorizontalBox::Slot()
					.Padding(2.f, 0.f)
					.AutoWidth()
					[
						FDialogueCharacterName_Customization::CreateValueWidget(SpeakerNamePropertyHandle.ToSharedRef(), Config->GetDialogueNameList())
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(2.f, 0.f)
					.AutoWidth()
					[
						ToAllTargetsPropertyHandle->CreatePropertyNameWidget()
					]
					+ SHorizontalBox::Slot()
					.Padding(2.f, 0.f)
					.AutoWidth()
					[
						ToAllTargetsPropertyHandle->CreatePropertyValueWidget()
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(2.f, 0.f)
					.AutoWidth()
					[
						TargetNamesPropertyHandle->CreatePropertyNameWidget()
					]
					+ SHorizontalBox::Slot()
					.Padding(2.f, 0.f)
					.AutoWidth()
					[
						TargetNamesPropertyHandle->CreatePropertyValueWidget()
					]
				]
				+ SVerticalBox::Slot()
				[
					TargetsWidget
				]
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
  			[
				DialogueSentencePropertyHandle->CreatePropertyValueWidget()
  			]
		];
}

void FDialogueSentenceEditData_Customization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	
}

void FDialogueCharacterName_Customization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	UGenDialogueSequenceConfigBase* Config = Cast<UGenDialogueSequenceConfigBase>(CustomizationUtils::GetOuter(StructPropertyHandle));

	HeaderRow.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		]
	.ValueContent()
		[
			FDialogueCharacterName_Customization::CreateValueWidget(StructPropertyHandle, Config->GetDialogueNameList())
		];
}

TSharedRef<SWidget> FDialogueCharacterName_Customization::CreateValueWidget(TSharedRef<class IPropertyHandle> StructPropertyHandle, TArray<TSharedPtr<FName>>& DialogueNameList)
{
	UGenDialogueSequenceConfigBase* Config = Cast<UGenDialogueSequenceConfigBase>(CustomizationUtils::GetOuter(StructPropertyHandle));
	TSharedPtr<IPropertyHandle> NamePropertyHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FDialogueCharacterName, Name));

	return SNew(SNameComboBox)
			.OptionsSource(&DialogueNameList)
			.InitiallySelectedItem([&]
			{
				FName SelectedName = CustomizationUtils::GetValue<FName>(NamePropertyHandle);
				for (const TSharedPtr<FName>& NameRef : DialogueNameList)
				{
					if (SelectedName == *NameRef.Get())
					{
						return NameRef;
					}
				}
				return TSharedPtr<FName>(nullptr);
			}())
			.OnSelectionChanged_Lambda([=](TSharedPtr<FName> Selection, ESelectInfo::Type SelectInfo)
			{
				if (SelectInfo == ESelectInfo::OnMouseClick || SelectInfo == ESelectInfo::OnKeyPress)
				{
					NamePropertyHandle->SetValue(*Selection.Get());
				}
			})
// 			.([=]
// 			{
// 				FName SelectedName = CustomizationUtils::GetValue<FName>(NamePropertyHandle);
// 				for (const TSharedPtr<FName>& NameRef : DialogueNameList)
// 				{
// 					if (SelectedName == *NameRef.Get())
// 					{
// 						return FSlateColor();
// 					}
// 				}
// 				return FSlateColor(FLinearColor::Red);
// 			})
			.ToolTipText_Lambda([=]()
			{
				FName SelectedName = CustomizationUtils::GetValue<FName>(NamePropertyHandle);
				for (const TSharedPtr<FName>& NameRef : DialogueNameList)
				{
					if (SelectedName == *NameRef.Get())
					{
						return FText::FromName(*NameRef);
					}
				}
				return FText::Format(LOCTEXT("名称无效", "[{0}]名称无效"), FText::FromName(SelectedName));
			})
			.IsEnabled(!StructPropertyHandle->IsEditConst());
}

#undef LOCTEXT_NAMESPACE
