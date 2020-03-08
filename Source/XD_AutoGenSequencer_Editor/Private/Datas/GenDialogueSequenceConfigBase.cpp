// Fill out your copyright notice in the Description page of Project Settings.


#include "Datas/GenDialogueSequenceConfigBase.h"
#include "Datas/DialogueStandPositionTemplate.h"
#include "Utils/AutoGenDialogueSettings.h"
#include "Preview/Sequence/PreviewDialogueSoundSequence.h"
#include "Datas/AutoGenDialogueAnimSet.h"
#include "Interface/XD_AutoGenDialogueInterface.h"
#include "Datas/AutoGenDialogueCameraSet.h"
#include "Datas/AutoGenDialogueCharacterSettings.h"
#include "Utils/GenDialogueSequenceEditor.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

FDialogueCharacterData::FDialogueCharacterData()
{
	
}

void UGenDialogueSequenceConfigBase::SyncInstanceData(const ADialogueStandPositionTemplate* Instance)
{
	for (int32 Idx = 0; Idx < DialogueCharacterDatas.Num(); ++Idx)
	{
		DialogueCharacterDatas[Idx].PositionOverride = Instance->StandPositions[Idx].StandPosition;
	}
}

TArray<FName> UGenDialogueSequenceConfigBase::GetCharacterNames() const
{
	TArray<FName> ValidNameList;
	for (const FDialogueCharacterData& DialogueCharacterData : DialogueCharacterDatas)
	{
		ValidNameList.Add(DialogueCharacterData.NameOverride);
	}
	return ValidNameList;
}

TArray<TSharedPtr<FName>>& UGenDialogueSequenceConfigBase::GetDialogueNameList()
{
	ReinitDialogueNameList();
	return DialogueNameList;
}

void UGenDialogueSequenceConfigBase::ReinitDialogueNameList()
{
	if (DialogueNameList.Num() != DialogueCharacterDatas.Num())
	{
		DialogueNameList.Reset(DialogueCharacterDatas.Num());
		for (const FDialogueCharacterData& Override : DialogueCharacterDatas)
		{
			DialogueNameList.Add(MakeShareable(new FName(Override.NameOverride)));
		}
	}
	else
	{
		for (int32 Idx = 0; Idx < DialogueNameList.Num(); ++Idx)
		{
			FName& CachedName = *DialogueNameList[Idx].Get();
			const FName& TargetName = DialogueCharacterDatas[Idx].NameOverride;
			if (CachedName != TargetName)
			{
				CachedName = TargetName;
			}
		}
	}
}

UGenDialogueSequenceConfigBase::UGenDialogueSequenceConfigBase(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PreviewDialogueSequence = CreateDefaultSubobject<UPreviewDialogueSoundSequence>(GET_MEMBER_NAME_CHECKED(UGenDialogueSequenceConfigBase, PreviewDialogueSequence));
	{
		PreviewDialogueSequence->Initialize();
		PreviewDialogueSequence->SetFlags(EObjectFlags::RF_Transactional);
		PreviewDialogueSequence->GetMovieScene()->SetFlags(EObjectFlags::RF_Public | EObjectFlags::RF_Transactional);
	}
}

void UGenDialogueSequenceConfigBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UGenDialogueSequenceConfigBase, DialogueStationTemplate))
	{
		if (DialogueStationTemplate)
		{
			const ADialogueStandPositionTemplate* DialogueStandPositionTemplate = DialogueStationTemplate.GetDefaultObject();
			DialogueCharacterDatas.SetNumZeroed(DialogueStandPositionTemplate->StandPositions.Num());
			SyncInstanceData(DialogueStandPositionTemplate);

			FDialogueCharacterData DefaultDialogueCharacterData;
			for (int32 Idx = 0; Idx < DialogueCharacterDatas.Num(); ++Idx)
			{
				const FDialogueStandPosition& DialogueStandPosition = DialogueStandPositionTemplate->StandPositions[Idx];
				FDialogueCharacterData& DialogueCharacterData = DialogueCharacterDatas[Idx];
				if (DialogueCharacterData.NameOverride.IsNone())
				{
					DialogueCharacterData.NameOverride = DialogueStandPosition.StandName;
				}
				if (DialogueCharacterData.TypeOverride == nullptr)
				{
					DialogueCharacterData.TypeOverride = DialogueStandPosition.PreviewCharacter ? DialogueStandPosition.PreviewCharacter : DialogueStandPositionTemplate->PreviewCharacter;
				}
				if (DialogueCharacterData.DialogueAnimSet == nullptr)
				{
					DialogueCharacterData.DialogueAnimSet = GetDefault<UAutoGenDialogueSettings>()->DefaultAutoGenDialogueAnimSet.LoadSynchronous();
				}
			}

			DialogueNameList.Empty();
			GetDialogueNameList();
		}
		ReinitDialogueNameList();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(FDialogueCharacterData, NameOverride))
	{
		ReinitDialogueNameList();
	}
}

const FDialogueCharacterData* UGenDialogueSequenceConfigBase::FindDialogueCharacterData(const FName& Name) const
{
	return DialogueCharacterDatas.FindByPredicate([&](const FDialogueCharacterData& E){ return E.NameOverride == Name; });
}

bool UGenDialogueSequenceConfigBase::IsConfigValid(TArray<FText>& ErrorMessages) const
{
	bool bIsValid = true;
	if (DialogueStationTemplate == nullptr)
	{
		ErrorMessages.Add(LOCTEXT("站位模板为空", "站位模板为空"));
		bIsValid &= false;
	}
	else
	{
		if (UAutoGenDialogueCameraSet* AutoGenDialogueCameraSet = GetAutoGenDialogueCameraSet())
		{
			bIsValid &= AutoGenDialogueCameraSet->IsValid(ErrorMessages);
		}
		else
		{
			ErrorMessages.Add(LOCTEXT("站位中镜头模板集未配置", "站位中镜头模板集未配置"));
			bIsValid &= false;
		}
	}
	TSubclassOf<UAutoGenDialogueAnimSetBase> AnimSetType = GetAnimSetType();
	if (AnimSetType == nullptr)
	{
		ErrorMessages.Add(LOCTEXT("允许的AnimSet类型未设置", "允许的AnimSet类型未设置"));
		bIsValid &= false;
	}	
	
	TArray<FName> VisitedName;
	TArray<UAutoGenDialogueAnimSetBase*> VisitedAnimSets;
	for (const FDialogueCharacterData& E : DialogueCharacterDatas)
	{
		if (E.DialogueAnimSet == nullptr || !E.DialogueAnimSet->IsA(AnimSetType))
		{
			ErrorMessages.Add(FText::Format(LOCTEXT("AnimSet类型错误", "角色[{0}] 对白动画集类型错误，应该为 [{1}]"), FText::FromName(E.NameOverride), AnimSetType->GetDisplayNameText()));
			bIsValid &= false;
		}
		else if (!VisitedAnimSets.Contains(E.DialogueAnimSet))
		{
			VisitedAnimSets.Add(E.DialogueAnimSet);
			TArray<FText> AnimSetErrorMessages;
			bool bIsAnimSetValid = E.DialogueAnimSet->IsAnimSetValid(AnimSetErrorMessages);
			if (bIsAnimSetValid == false)
			{
				ErrorMessages.Add(FText::Format(LOCTEXT("AnimSet配置错误", "角色[{0}] 对白动画集[{1}] 配置错误："), FText::FromName(E.NameOverride), FText::FromString(E.DialogueAnimSet->GetName())));
				ErrorMessages.Append(AnimSetErrorMessages);
				bIsValid &= false;
			}
		}
		if (!VisitedName.Contains(E.NameOverride))
		{
			VisitedName.Add(E.NameOverride);
			if (DialogueCharacterDatas.FilterByPredicate([&](const FDialogueCharacterData& E2) {return E2.NameOverride == E.NameOverride; }).Num() >= 2)
			{
				ErrorMessages.Add(FText::Format(LOCTEXT("对白角色数据重名", "角色名[{0}]重复"), FText::FromString(E.NameOverride.ToString())));
				bIsValid &= false;
			}
		}
		if (E.TypeOverride == nullptr)
		{
			ErrorMessages.Add(FText::Format(LOCTEXT("对白角色类型未配置", "角色[{0}] 类型未配置"), FText::FromString(E.NameOverride.ToString())));
			bIsValid &= false;
		}
		else
		{
			if (!E.TypeOverride->ImplementsInterface(UXD_AutoGenDialogueInterface::StaticClass()))
			{
				ErrorMessages.Add(FText::Format(LOCTEXT("对白角色类型未实现DialogueInterface接口", "角色[{0}] 类型未实现DialogueInterface接口"), FText::FromName(E.NameOverride)));
				bIsValid &= false;
			}
		}
		if (TSubclassOf<UAutoGenDialogueCharacterSettings> CharacterSettingsType = GetCharacterSettingsType())
		{
			if (E.CharacterSettings == nullptr || !E.CharacterSettings->IsA(CharacterSettingsType))
			{
				ErrorMessages.Add(FText::Format(LOCTEXT("CharacterSettings类型错误", "角色[{0}] 对白角色配置类型错误，应该为 [{1}]"), FText::FromName(E.NameOverride), CharacterSettingsType->GetDisplayNameText()));
				bIsValid &= false;
			}
			else
			{
				const FName LookAtTargetPropertyName = E.CharacterSettings->LookAtTargetPropertyName;
				if (FProperty* LookAtTargetProperty = E.TypeOverride->FindPropertyByName(LookAtTargetPropertyName))
				{
					if (!LookAtTargetProperty->IsA<FSoftObjectProperty>())
					{
						ErrorMessages.Add(FText::Format(LOCTEXT("对白角色LookAtTargetProperty属性错误", "角色[{0}] 类型[{1}]属性类型需要为SoftObject"), FText::FromName(E.NameOverride), FText::FromName(LookAtTargetPropertyName)));
						bIsValid &= false;
					}
				}
				else
				{
					ErrorMessages.Add(FText::Format(LOCTEXT("对白角色未添加LookAtTargetProperty属性", "角色[{0}] 类型未添加[{1}]属性"), FText::FromName(E.NameOverride), FText::FromName(LookAtTargetPropertyName)));
					bIsValid &= false;
				}
			}
		}
	}
	return bIsValid;
}

TSubclassOf<UAutoGenDialogueAnimSetBase> UGenDialogueSequenceConfigBase::GetAnimSetType() const
{
	return nullptr;
}

TSubclassOf<UAutoGenDialogueCharacterSettings> UGenDialogueSequenceConfigBase::GetCharacterSettingsType() const
{
	return UAutoGenDialogueCharacterSettings::StaticClass();
}

void UGenDialogueSequenceConfigBase::GeneratePreviewSequence()
{
	FGenDialogueSequenceEditor::Get().GeneratePreviewSequence();
}

void UGenDialogueSequenceConfigBase::GenerateDialogueSequence()
{
	FGenDialogueSequenceEditor::Get().GenerateDialogueSequence();
}

UAutoGenDialogueCameraSet* UGenDialogueSequenceConfigBase::GetAutoGenDialogueCameraSet() const
{
	return DialogueStationTemplate.GetDefaultObject()->AutoGenDialogueCameraSet;
}

ULevelSequence* UGenDialogueSequenceConfigBase::GetOwingLevelSequence() const
{
	return GetTypedOuter<ULevelSequence>();
}

#undef LOCTEXT_NAMESPACE
