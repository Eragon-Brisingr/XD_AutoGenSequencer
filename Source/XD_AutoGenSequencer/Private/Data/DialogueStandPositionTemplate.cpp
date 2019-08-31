// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueStandPositionTemplate.h"
#include "GameFramework/Character.h"
#include "Components/ChildActorComponent.h"
#include "Components/BillboardComponent.h"
#include "Engine/BlueprintGeneratedClass.h"


#if WITH_EDITOR
ADialogueStandPositionTemplate::ADialogueStandPositionTemplate()
{
	bIsEditorOnlyActor = true;
	bHidden = false;

	StandPositions.Add(FDialogueStandPosition(TEXT("Role"), nullptr));
	StandPositions.Add(FDialogueStandPosition(TEXT("Target1"), nullptr));
}

void ADialogueStandPositionTemplate::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		FName PropertyName = PropertyChangedEvent.Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ADialogueStandPositionTemplate, StandPositions) || PropertyName == GET_MEMBER_NAME_CHECKED(ADialogueStandPositionTemplate, PreviewCharacter))
		{
			if (!IsTemplate())
			{
				ClearInvalidPreviewCharacter();
				CreateAllTemplatePreviewCharacter();
			}
		}
		if (!IsTemplate() && PropertyName != TEXT("ActorLabel"))
		{
			ApplyStandPositionsToDefault();
		}
	}
}

void ADialogueStandPositionTemplate::OnConstruction(const FTransform& Transform)
{
	if (!bSpawnedPreviewCharacter)
	{
		bSpawnedPreviewCharacter = true;
		CreateAllTemplatePreviewCharacter();
	}
	for (FDialogueStandPosition& StandPosition : StandPositions)
	{
		if (UChildActorComponent* ChildActorComponent = StandPosition.PreviewCharacterInstance)
		{
			ChildActorComponent->SetRelativeTransform(StandPosition.StandPosition);
			if (ACharacter* Character = Cast<ACharacter>(ChildActorComponent->GetChildActor()))
			{
				Character->SetActorRelativeLocation(FVector(0.f, 0.f, Character->GetDefaultHalfHeight()));
			}
		}
	}

	OnInstanceChanged.ExecuteIfBound();
}

UChildActorComponent* ADialogueStandPositionTemplate::CreateChildActorComponent()
{
	UChildActorComponent* ChildActorComponent = NewObject<UChildActorComponent>(this, NAME_None, RF_Transient);
	ChildActorComponent->RegisterComponent();
	AddOwnedComponent(ChildActorComponent);
	ChildActorComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	PreviewCharacters.Add(ChildActorComponent);
	return ChildActorComponent;
}

void ADialogueStandPositionTemplate::CreateAllTemplatePreviewCharacter()
{
	for (FDialogueStandPosition& StandPosition : StandPositions)
	{
		UChildActorComponent* ChildActorComponent = StandPosition.PreviewCharacterInstance;
		if (!ChildActorComponent)
		{
			ChildActorComponent = CreateChildActorComponent();
			StandPosition.PreviewCharacterInstance = ChildActorComponent;
		}

		UClass* CharacterClass = StandPosition.PreviewCharacter ? StandPosition.PreviewCharacter : PreviewCharacter;
		if (CharacterClass && ChildActorComponent->GetChildActorClass() != CharacterClass)
		{
			ChildActorComponent->SetChildActorClass(CharacterClass);
			ChildActorComponent->GetChildActor()->SetFlags(RF_Transient);

			ChildActorComponent->GetChildActor()->SetActorLabel(StandPosition.StandName.ToString());
		}

		ChildActorComponent->SetRelativeTransform(StandPosition.StandPosition);
		if (ACharacter* Character = Cast<ACharacter>(ChildActorComponent->GetChildActor()))
		{
			Character->SetActorRelativeLocation(FVector(0.f, 0.f, Character->GetDefaultHalfHeight()));
		}
	}
}

void ADialogueStandPositionTemplate::ClearInvalidPreviewCharacter()
{
	TSet<UChildActorComponent*> ValidChildActorComponent;
	for (FDialogueStandPosition& StandPosition : StandPositions)
	{
		if (StandPosition.PreviewCharacterInstance)
		{
			ValidChildActorComponent.Add(StandPosition.PreviewCharacterInstance);
		}
	}

	for (UChildActorComponent* InvalidActorComponent : TSet<UChildActorComponent*>(PreviewCharacters).Difference(ValidChildActorComponent))
	{
		PreviewCharacters.Remove(InvalidActorComponent);
		InvalidActorComponent->DestroyComponent(true);
	}
}

void ADialogueStandPositionTemplate::ApplyStandPositionsToDefault()
{
	if (!IsTemplate())
	{
		// TODO:这么做会导致这Actor还存在在关卡里的话引起关卡保存报错，需查明原因
		if (UBlueprintGeneratedClass* BlueprintGeneratedClass = Cast<UBlueprintGeneratedClass>(GetClass()))
		{
			BlueprintGeneratedClass->Modify(true);
			ADialogueStandPositionTemplate* Default = GetClass()->GetDefaultObject<ADialogueStandPositionTemplate>();
			Default->StandPositions = StandPositions;
		}
	}
}

#endif
