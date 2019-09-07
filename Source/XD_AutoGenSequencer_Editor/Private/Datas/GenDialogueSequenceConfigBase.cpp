// Fill out your copyright notice in the Description page of Project Settings.


#include "GenDialogueSequenceConfigBase.h"
#include "DialogueStandPositionTemplate.h"
#include "AutoGenDialogueSettings.h"
#include "PreviewDialogueSoundSequence.h"

FDialogueCharacterData::FDialogueCharacterData()
{
	
}

#if WITH_EDITORONLY_DATA
void FDialogueStationInstance::SyncInstanceData(const ADialogueStandPositionTemplate* Instance)
{
	for (int32 Idx = 0; Idx < DialogueCharacterDatas.Num(); ++Idx)
	{
		DialogueCharacterDatas[Idx].PositionOverride = Instance->StandPositions[Idx].StandPosition;
	}
}

bool FDialogueStationInstance::IsValid() const
{
	if (DialogueStationTemplate == nullptr)
	{
		return false;
	}
	if (DialogueCharacterDatas.ContainsByPredicate([](const FDialogueCharacterData& E) {return E.DialogueAnimSet == nullptr; }))
	{
		return false;
	}
	return true;
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

TArray<TSharedPtr<FString>>& FDialogueStationInstance::GetDialogueNameList()
{
	if (DialogueNameList.Num() == 0)
	{
		for (const FDialogueCharacterData& Override : DialogueCharacterDatas)
		{
			DialogueNameList.Add(MakeShareable(new FString(Override.NameOverride.ToString())));
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
			*(DialogueNameList[Idx].Get()) = DialogueCharacterDatas[Idx].NameOverride.ToString();
		}
		else
		{
			DialogueNameList[Idx] = MakeShareable(new FString(DialogueCharacterDatas[Idx].NameOverride.ToString()));
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
					DialogueCharacterData.TalkAnimSlotName = TEXT("DefaultSlot");
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

bool UGenDialogueSequenceConfigBase::IsConfigValid() const
{
	if (!DialogueStation.IsValid())
	{
		return false;
	}
	return true;
}

#endif
