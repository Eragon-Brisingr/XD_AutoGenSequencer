// Fill out your copyright notice in the Description page of Project Settings.


#include "GenDialogueSequenceConfigBase.h"
#include "DialogueStandPositionTemplate.h"
#include "AutoGenDialogueSettings.h"
#include "PreviewDialogueSoundSequence.h"
#include "AutoGenDialogueAnimSet.h"
#include "DialogueInterface.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

FDialogueCharacterData::FDialogueCharacterData()
{
	
}

void FDialogueStationInstance::SyncInstanceData(const ADialogueStandPositionTemplate* Instance)
{
	for (int32 Idx = 0; Idx < DialogueCharacterDatas.Num(); ++Idx)
	{
		DialogueCharacterDatas[Idx].PositionOverride = Instance->StandPositions[Idx].StandPosition;
	}
}

TArray<FName> FDialogueStationInstance::GetCharacterNames() const
{
	TArray<FName> ValidNameList;
	for (const FDialogueCharacterData& DialogueCharacterData : DialogueCharacterDatas)
	{
		ValidNameList.Add(DialogueCharacterData.NameOverride);
	}
	return ValidNameList;
}

TSharedPtr<FName> FDialogueStationInstance::InvalidDialogueName = MakeShareable(new FName(TEXT("无效的角色名")));

TArray<TSharedPtr<FName>>& FDialogueStationInstance::GetDialogueNameList()
{
	if (DialogueNameList.Num() == 0)
	{
		for (const FDialogueCharacterData& Override : DialogueCharacterDatas)
		{
			DialogueNameList.Add(MakeShareable(new FName(Override.NameOverride)));
		}
	}
	return DialogueNameList;
}

void FDialogueStationInstance::ReinitDialogueNameList()
{
	DialogueNameList.SetNumZeroed(DialogueCharacterDatas.Num());
	for (int32 Idx = 0; Idx < DialogueNameList.Num(); ++Idx)
	{
		if (DialogueNameList[Idx].IsValid())
		{
			*(DialogueNameList[Idx].Get()) = DialogueCharacterDatas[Idx].NameOverride;
		}
		else
		{
			DialogueNameList[Idx] = MakeShareable(new FName(DialogueCharacterDatas[Idx].NameOverride));
		}
	}
}

UGenDialogueSequenceConfigBase::UGenDialogueSequenceConfigBase(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PreviewDialogueSoundSequence = CreateDefaultSubobject<UPreviewDialogueSoundSequence>(GET_MEMBER_NAME_CHECKED(UGenDialogueSequenceConfigBase, PreviewDialogueSoundSequence));
	{
		PreviewDialogueSoundSequence->Initialize();
		const EObjectFlags Flags = EObjectFlags::RF_Public | EObjectFlags::RF_Standalone | EObjectFlags::RF_Transactional;
		PreviewDialogueSoundSequence->SetFlags(Flags);
		PreviewDialogueSoundSequence->GetMovieScene()->SetFlags(Flags);
	}
}

void UGenDialogueSequenceConfigBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(FDialogueStationInstance, DialogueStationTemplate))
	{
		if (DialogueStation.DialogueStationTemplate)
		{
			const ADialogueStandPositionTemplate* DialogueStandPositionTemplate = DialogueStation.DialogueStationTemplate.GetDefaultObject();
			DialogueStation.DialogueCharacterDatas.SetNumZeroed(DialogueStandPositionTemplate->StandPositions.Num());
			DialogueStation.SyncInstanceData(DialogueStandPositionTemplate);

			FDialogueCharacterData DefaultDialogueCharacterData;
			for (int32 Idx = 0; Idx < DialogueStation.DialogueCharacterDatas.Num(); ++Idx)
			{
				const FDialogueStandPosition& DialogueStandPosition = DialogueStandPositionTemplate->StandPositions[Idx];
				FDialogueCharacterData& DialogueCharacterData = DialogueStation.DialogueCharacterDatas[Idx];
				if (DialogueCharacterData.NameOverride.IsNone())
				{
					DialogueCharacterData.NameOverride = DialogueStandPosition.StandName;
				}
				if (DialogueCharacterData.TypeOverride == nullptr)
				{
					DialogueCharacterData.TypeOverride = DialogueStandPosition.PreviewCharacter ? DialogueStandPosition.PreviewCharacter : DialogueStandPositionTemplate->PreviewCharacter;
				}
				if (DialogueCharacterData.TalkAnimSlotName.IsNone())
				{
					DialogueCharacterData.TalkAnimSlotName = DefaultDialogueCharacterData.TalkAnimSlotName;
				}
				if (DialogueCharacterData.LookAtTargetPropertyName.IsNone())
				{
					DialogueCharacterData.LookAtTargetPropertyName = DefaultDialogueCharacterData.LookAtTargetPropertyName;
				}
				if (DialogueCharacterData.DialogueAnimSet == nullptr)
				{
					DialogueCharacterData.DialogueAnimSet = GetDefault<UAutoGenDialogueSettings>()->DefaultAutoGenDialogueAnimSet.LoadSynchronous();
				}
			}

			DialogueStation.DialogueNameList.Empty();
			DialogueStation.GetDialogueNameList();
		}
		DialogueStation.ReinitDialogueNameList();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(FDialogueCharacterData, NameOverride))
	{
		DialogueStation.ReinitDialogueNameList();
	}
}

bool UGenDialogueSequenceConfigBase::IsConfigValid(TArray<FText>& ErrorMessages) const
{
	bool bIsSucceed = true;
	if (DialogueStation.DialogueStationTemplate == nullptr)
	{
		ErrorMessages.Add(LOCTEXT("对话模板为空", "对话模板为空"));
		bIsSucceed &= false;
	}
	TSubclassOf<UAutoGenDialogueAnimSetBase> AnimSetType = GetAnimSetType();
	if (AnimSetType == nullptr)
	{
		ErrorMessages.Add(LOCTEXT("允许的AnimSet类型未设置", "允许的AnimSet类型未设置"));
		bIsSucceed &= false;
	}
	TArray<FName> VisitedName;
	TArray<UAutoGenDialogueAnimSetBase*> VisitedAnimSets;
	for (const FDialogueCharacterData& E : DialogueStation.DialogueCharacterDatas)
	{
		if (E.DialogueAnimSet == nullptr || !E.DialogueAnimSet->IsA(AnimSetType))
		{
			ErrorMessages.Add(FText::Format(LOCTEXT("AnimSet类型错误", "角色[{0}] 对白动画集类型错误，应该为 [{1}]"), FText::FromString(E.NameOverride.ToString()), AnimSetType->GetDisplayNameText()));
			bIsSucceed &= false;
		}
		else if (!VisitedAnimSets.Contains(E.DialogueAnimSet))
		{
			VisitedAnimSets.Add(E.DialogueAnimSet);
			TArray<FText> AnimSetErrorMessages;
			bool bIsAnimSetValid = E.DialogueAnimSet->IsAnimSetValid(AnimSetErrorMessages);
			if (bIsAnimSetValid == false)
			{
				ErrorMessages.Add(FText::Format(LOCTEXT("AnimSet配置错误", "角色[{0}] 对白动画集[{1}] 配置错误："), FText::FromString(E.NameOverride.ToString()), FText::FromString(E.DialogueAnimSet->GetName())));
				ErrorMessages.Append(AnimSetErrorMessages);
				bIsSucceed &= false;
			}
		}
		if (!VisitedName.Contains(E.NameOverride))
		{
			VisitedName.Add(E.NameOverride);
			if (DialogueStation.DialogueCharacterDatas.FilterByPredicate([&](const FDialogueCharacterData& E2) {return E2.NameOverride == E.NameOverride; }).Num() >= 2)
			{
				ErrorMessages.Add(FText::Format(LOCTEXT("对白角色数据重名", "角色名[{0}]重复"), FText::FromString(E.NameOverride.ToString())));
				bIsSucceed &= false;
			}
		}
		if (E.TypeOverride == nullptr)
		{
			ErrorMessages.Add(FText::Format(LOCTEXT("对白角色数据重名", "角色[{0}] 类型未配置"), FText::FromString(E.NameOverride.ToString())));
			bIsSucceed &= false;
		}
		else if (!E.TypeOverride->ImplementsInterface(UDialogueInterface::StaticClass()))
		{
			ErrorMessages.Add(FText::Format(LOCTEXT("对白角色数据重名", "角色[{0}] 类型未实现DialogueInterface接口"), FText::FromString(E.NameOverride.ToString())));
			bIsSucceed &= false;
		}
	}
	return bIsSucceed;
}

TSubclassOf<UAutoGenDialogueAnimSetBase> UGenDialogueSequenceConfigBase::GetAnimSetType() const
{
	return nullptr;
}

#undef LOCTEXT_NAMESPACE
