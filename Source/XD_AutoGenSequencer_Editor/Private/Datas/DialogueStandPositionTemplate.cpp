// Fill out your copyright notice in the Description page of Project Settings.


#include "Datas/DialogueStandPositionTemplate.h"
#include <GameFramework/Character.h>
#include <Components/ChildActorComponent.h>
#include <Components/BillboardComponent.h>

#include "XD_AutoGenSequencer_Editor.h"
#include "Utils/AutoGenDialogueSettings.h"
#include "Utils/StandTemplateEditor.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

ADialogueStandPositionTemplate::ADialogueStandPositionTemplate()
{
	bIsEditorOnlyActor = true;
	SetHidden(false);

	StandPositions.Add(FDialogueStandPosition(TEXT("Role"), nullptr, FTransform(FRotator(0.f, 180.f, 0.f), FVector(100.f, 0.f, 0.f))));
	StandPositions.Add(FDialogueStandPosition(TEXT("Target1"), nullptr, FTransform(FVector(-100.f, 0.f, 0.f))));

	PreviewCharacter = ACharacter::StaticClass();
	AutoGenDialogueCameraSet = GetDefault<UAutoGenDialogueSettings>()->DefaultAutoGenDialogueCameraSet.LoadSynchronous();
}

void ADialogueStandPositionTemplate::PreEditChange(FProperty* PropertyThatWillChange)
{
	Super::PreEditChange(PropertyThatWillChange);
}

void ADialogueStandPositionTemplate::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		FName PropertyName = PropertyChangedEvent.Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ADialogueStandPositionTemplate, StandPositions))
		{
			CreateAllTemplatePreviewCharacter();
			TSet<UChildActorComponent*> PreComponents{ PreviewCharacters };
			PreviewCharacters.Empty();
			for (FDialogueStandPosition& StandPosition : StandPositions)
			{
				UChildActorComponent* ChildActorComponent = StandPosition.PreviewCharacterInstance;
				PreviewCharacters.Add(StandPosition.PreviewCharacterInstance);
			}
			for (UChildActorComponent* OldComponent : PreComponents.Difference(TSet<UChildActorComponent*>(PreviewCharacters)))
			{
				OldComponent->DestroyComponent();
			}
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(ADialogueStandPositionTemplate, PreviewCharacter) || PropertyName == GET_MEMBER_NAME_CHECKED(FDialogueStandPosition, PreviewCharacter))
		{
			CreateAllTemplatePreviewCharacter();
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(FDialogueStandPosition, StandPosition))
		{
			UpdateToStandLocation();
		}
	}
	UpdateToStandLocation();

	OnInstanceChanged.ExecuteIfBound();
}

void ADialogueStandPositionTemplate::OnConstruction(const FTransform& Transform)
{
	UpdateToStandLocation();

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
		if (ChildActorComponent->GetChildActorClass() != CharacterClass)
		{
			ChildActorComponent->SetChildActorClass(CharacterClass);
			if (AActor* Actor = ChildActorComponent->GetChildActor())
			{
				Actor->SetFlags(RF_Transient);
				Actor->SetActorLabel(StandPosition.StandName.ToString());
			}
		}

		ChildActorComponent->SetRelativeTransform(StandPosition.StandPosition);
		if (ACharacter* Character = Cast<ACharacter>(ChildActorComponent->GetChildActor()))
		{
			Character->SetActorRelativeLocation(FVector(0.f, 0.f, Character->GetDefaultHalfHeight()));
		}
	}
}

void ADialogueStandPositionTemplate::UpdateToStandLocation()
{
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
}

UDialogueStandPositionTemplateFactory::UDialogueStandPositionTemplateFactory()
{
	SupportedClass = UDialogueStandPositionTemplateAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UDialogueStandPositionTemplateFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	UDialogueStandPositionTemplateAsset* Asset = NewObject<UDialogueStandPositionTemplateAsset>(InParent, Name, Flags);
	Asset->Template = NewObject<ADialogueStandPositionTemplate>(Asset, Name, Flags);
	return Asset;
}

FText UDialogueStandPositionTemplateFactory::GetDisplayName() const
{
	return LOCTEXT("创建对白站位模板", "对白站位模板");
}

FText FAssetTypeActions_DialogueStandPositionTemplate::GetName() const
{
	return LOCTEXT("对白站位模板", "对白站位模板");
}

UClass* FAssetTypeActions_DialogueStandPositionTemplate::GetSupportedClass() const
{
	return UDialogueStandPositionTemplateAsset::StaticClass();
}

FColor FAssetTypeActions_DialogueStandPositionTemplate::GetTypeColor() const
{
	return FColor::Black;
}

uint32 FAssetTypeActions_DialogueStandPositionTemplate::GetCategories()
{
	return FXD_AutoGenSequencer_EditorModule::AutoGenDialogueSequence_AssetCategory;
}

void FAssetTypeActions_DialogueStandPositionTemplate::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	for (UObject* Object : InObjects)
	{
		if (UDialogueStandPositionTemplateAsset* Asset = Cast<UDialogueStandPositionTemplateAsset>(Object))
		{
			TSharedRef<FStandTemplateEditor> EditorToolkit = MakeShareable(new FStandTemplateEditor());
			EditorToolkit->InitStandTemplateEditor(Mode, EditWithinLevelEditor, Asset);
		}
	}
}

#undef LOCTEXT_NAMESPACE
