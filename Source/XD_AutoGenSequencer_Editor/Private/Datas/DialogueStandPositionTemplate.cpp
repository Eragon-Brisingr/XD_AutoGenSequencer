// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueStandPositionTemplate.h"
#include "GameFramework/Character.h"
#include "Components/ChildActorComponent.h"
#include "Components/BillboardComponent.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "XD_AutoGenSequencer_Editor.h"
#include "KismetCompilerModule.h"
#include "KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

ADialogueStandPositionTemplate::ADialogueStandPositionTemplate()
{
#if WITH_EDITOR
	bIsEditorOnlyActor = true;
	bHidden = false;

	StandPositions.Add(FDialogueStandPosition(TEXT("Role"), nullptr, FTransform(FVector(100.f, 0.f, 0.f))));
	StandPositions.Add(FDialogueStandPosition(TEXT("Target1"), nullptr, FTransform(FVector(-100.f, 0.f, 0.f))));
#endif

	PreviewCharacter = ACharacter::StaticClass();
}

#if WITH_EDITOR
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

UDialogueStandPositionTemplateFactory::UDialogueStandPositionTemplateFactory()
{
	SupportedClass = ADialogueStandPositionTemplate::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UDialogueStandPositionTemplateFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	UClass* BlueprintClass = nullptr;
	UClass* BlueprintGeneratedClass = nullptr;

	IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
	KismetCompilerModule.GetBlueprintTypesForClass(ADialogueStandPositionTemplate::StaticClass(), BlueprintClass, BlueprintGeneratedClass);

	return FKismetEditorUtilities::CreateBlueprint(ADialogueStandPositionTemplate::StaticClass(), InParent, Name, EBlueprintType::BPTYPE_Normal, BlueprintClass, BlueprintGeneratedClass);
}

FText UDialogueStandPositionTemplateFactory::GetDisplayName() const
{
	return LOCTEXT("创建对白站位模板", "对白站位模板");
}

uint32 UDialogueStandPositionTemplateFactory::GetMenuCategories() const
{
	return FXD_AutoGenSequencer_EditorModule::AutoGenDialogueSequence_AssetCategory;
}

#endif

#undef LOCTEXT_NAMESPACE
