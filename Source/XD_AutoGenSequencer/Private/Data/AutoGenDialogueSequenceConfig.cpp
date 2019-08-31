// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoGenDialogueSequenceConfig.h"
#include "Sound/DialogueWave.h"
#include "DialogueStandPositionTemplate.h"
#include "DialogueSentence.h"

void FDialogueStationInstance::SyncInstanceData(const ADialogueStandPositionTemplate* Instance)
{
	for (int32 Idx = 0; Idx < DialogueStationTemplateOverride.Num(); ++Idx)
	{
		DialogueStationTemplateOverride[Idx].PositionOverride = Instance->StandPositions[Idx].StandPosition;
	}
}

bool FDialogueStationInstance::IsValid() const
{
	return DialogueStationTemplate ? true : false;
}

TArray<FName> FDialogueStationInstance::GetCharacterNames() const
{
	TArray<FName> ValidNameList;
	for (const FDialogueStationInstanceOverride& Override : DialogueStationTemplateOverride)
	{
		ValidNameList.Add(Override.NameOverride);
	}
	return ValidNameList;
}

#if WITH_EDITORONLY_DATA
TArray<TSharedPtr<FString>>& FDialogueStationInstance::GetDialogueNameList()
{
	if (DialogueNameList.Num() == 0)
	{
		for (const FDialogueStationInstanceOverride& Override : DialogueStationTemplateOverride)
		{
			DialogueNameList.Add(MakeShareable(new FString(Override.NameOverride.ToString())));
		}
	}
	return DialogueNameList;
}

void FDialogueStationInstance::ReinitDialogueNameList()
{
	DialogueNameList.SetNumZeroed(DialogueStationTemplateOverride.Num());
	for (int32 Idx = 0; Idx < DialogueNameList.Num(); ++Idx)
	{
		if (DialogueNameList[Idx].IsValid())
		{
			*(DialogueNameList[Idx].Get()) = DialogueStationTemplateOverride[Idx].NameOverride.ToString();
		}
		else
		{
			DialogueNameList[Idx] = MakeShareable(new FString(DialogueStationTemplateOverride[Idx].NameOverride.ToString()));
		}
	}
}
#endif

USoundBase* FDialogueSentenceEditData::GetDefaultDialogueSound() const
{
	return DialogueSentence->SentenceWave;
}

#if WITH_EDITOR
void UAutoGenDialogueSequenceConfig::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(FDialogueStationInstance, DialogueStationTemplate))
	{
		if (DialogueStation.DialogueStationTemplate)
		{
			const ADialogueStandPositionTemplate* DialogueStandPositionTemplate = DialogueStation.DialogueStationTemplate.GetDefaultObject();
			DialogueStation.DialogueStationTemplateOverride.SetNumZeroed(DialogueStandPositionTemplate->StandPositions.Num());
			DialogueStation.SyncInstanceData(DialogueStandPositionTemplate);
			for (int32 Idx = 0; Idx < DialogueStation.DialogueStationTemplateOverride.Num(); ++Idx)
			{
				const FDialogueStandPosition& DialogueStandPosition = DialogueStandPositionTemplate->StandPositions[Idx];
				if (DialogueStation.DialogueStationTemplateOverride[Idx].NameOverride.IsNone())
				{
					DialogueStation.DialogueStationTemplateOverride[Idx].NameOverride = DialogueStandPosition.StandName;
				}
				if (DialogueStation.DialogueStationTemplateOverride[Idx].TypeOverride == nullptr)
				{
					DialogueStation.DialogueStationTemplateOverride[Idx].TypeOverride = DialogueStandPosition.PreviewCharacter ? DialogueStandPosition.PreviewCharacter : DialogueStandPositionTemplate->PreviewCharacter;
				}
			}

			DialogueStation.DialogueNameList.Empty();
			DialogueStation.GetDialogueNameList();
		}
		DialogueStation.ReinitDialogueNameList();
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(FDialogueStationInstanceOverride, NameOverride))
	{
		DialogueStation.ReinitDialogueNameList();
	}
}
#endif

const FDialogueStationInstanceOverride* UAutoGenDialogueSequenceConfig::GetStationOverrideDataBySentence(const FDialogueSentenceEditData& DialogueSentenceEditData) const
{	
	// TODO: 通过Character配置的DialogueVoice找到站位的名字
	if (const FDialogueStationInstanceOverride* DialogueStationInstanceOverride = DialogueStation.DialogueStationTemplateOverride.FindByPredicate([&](const FDialogueStationInstanceOverride& E) {return E.NameOverride == DialogueSentenceEditData.SpeakerName.GetName(); }))
	{
		return DialogueStationInstanceOverride;
	}
	checkNoEntry();
	return nullptr;
}

FName UAutoGenDialogueSequenceConfig::GetSpeakerNameBySentence(const FDialogueSentenceEditData& DialogueSentenceEditData) const
{
	return GetStationOverrideDataBySentence(DialogueSentenceEditData)->NameOverride;
}

bool UAutoGenDialogueSequenceConfig::IsConfigValid() const
{
	if (!DialogueStation.IsValid())
	{
		return false;
	}

	TArray<FName> ValidNameList = DialogueStation.GetCharacterNames();
	for (const FDialogueSentenceEditData& Data : DialogueSentenceEditDatas)
	{
		if (!IsDialogueSentenceEditDataValid(Data, ValidNameList))
		{
			return false;
		}
	}
	return true;
}

bool UAutoGenDialogueSequenceConfig::IsDialogueSentenceEditDataValid(const FDialogueSentenceEditData &Data, const TArray<FName>& ValidNameList) const
{
	if (!Data.DialogueSentence)
	{
		return false;
	}
	if (!Data.DialogueSentence->SentenceWave)
	{
		return false;
	}
	if (Data.TargetNames.Contains(Data.SpeakerName))
	{
		return false;
	}
	if (TSet<FDialogueCharacterName>(Data.TargetNames).Num() != Data.TargetNames.Num())
	{
		return false;
	}
	if (!ValidNameList.Contains(Data.SpeakerName.GetName()))
	{
		return false;
	}
	for (const FDialogueCharacterName& Name : Data.TargetNames)
	{
		if (!ValidNameList.Contains(Name.GetName()))
		{
			return false;
		}
	}
	return true;
}
