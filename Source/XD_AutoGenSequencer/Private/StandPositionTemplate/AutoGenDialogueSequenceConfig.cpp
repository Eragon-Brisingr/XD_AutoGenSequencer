// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoGenDialogueSequenceConfig.h"
#include "Sound/DialogueWave.h"
#include "DialogueStandPositionTemplate.h"

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

USoundWave* FDialogueSentenceEditData::GetDefaultDialogueSound() const
{
	return DialogueWave->ContextMappings[0].SoundWave;
}

UDialogueVoice* FDialogueSentenceEditData::GetDefualtDialogueSpeaker() const
{
	return DialogueWave->ContextMappings[0].Context.Speaker;
}

#if WITH_EDITOR
void UAutoGenDialogueSequenceConfig::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
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
				DialogueStation.DialogueStationTemplateOverride[Idx].NameOverride = DialogueStandPosition.StandName;
				DialogueStation.DialogueStationTemplateOverride[Idx].TypeOverride = DialogueStandPosition.PreviewCharacter ? DialogueStandPosition.PreviewCharacter : DialogueStandPositionTemplate->PreviewCharacter;
			}
		}
	}
}
#endif

const FDialogueStationInstanceOverride* UAutoGenDialogueSequenceConfig::GetStationOverrideDataBySentence(const FDialogueSentenceEditData& DialogueSentenceEditData) const
{	
	// TODO: 通过Character配置的DialogueVoice找到站位的名字
	if (const FDialogueStationInstanceOverride* DialogueStationInstanceOverride = DialogueStation.DialogueStationTemplateOverride.FindByPredicate([&](const FDialogueStationInstanceOverride& E) {return E.DialogueVoiceOverride == DialogueSentenceEditData.GetDefualtDialogueSpeaker(); }))
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
	return DialogueStation.IsValid();
}
