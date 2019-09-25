// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueStandPositionTemplate.h"
#include "GameFramework/Character.h"
#include "Components/ChildActorComponent.h"
#include "Components/BillboardComponent.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "XD_AutoGenSequencer_Editor.h"
#include "KismetCompilerModule.h"
#include "KismetEditorUtilities.h"
#include "Engine/Blueprint.h"
#include "BlueprintEditorUtils.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

ADialogueStandPositionTemplate::ADialogueStandPositionTemplate()
{
	bIsEditorOnlyActor = true;
	bHidden = false;

//	数组有BUG，设置默认值之后编辑器Diff到超出默认长度后的元素直接溢出了
// 	StandPositions.Add(FDialogueStandPosition(TEXT("Role"), nullptr, FTransform(FVector(100.f, 0.f, 0.f))));
// 	StandPositions.Add(FDialogueStandPosition(TEXT("Target1"), nullptr, FTransform(FVector(-100.f, 0.f, 0.f))));

	PreviewCharacter = ACharacter::StaticClass();
}

#if WITH_EDITOR
// 关闭Actor的修改回调是为了防止ChildActor重新生成
void ADialogueStandPositionTemplate::PreEditChange(UProperty* PropertyThatWillChange)
{
	UObject::PreEditChange(PropertyThatWillChange);
	//Super::PreEditChange(PropertyThatWillChange);
}

void ADialogueStandPositionTemplate::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);
	//Super::PostEditChangeProperty(PropertyChangedEvent);

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
		if (PropertyName != TEXT("ActorLabel"))
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
	if (IsInEditorMode())
	{
		if (UBlueprint * Blueprint = Cast<UBlueprint>(GetClass()->ClassGeneratedBy))
		{
			ADialogueStandPositionTemplate* Default = GetClass()->GetDefaultObject<ADialogueStandPositionTemplate>();
			Default->PreviewCharacter = PreviewCharacter;
			Default->StandPositions = StandPositions;

			for (FDialogueStandPosition& StandPos : Default->StandPositions)
			{
				StandPos.PreviewCharacterInstance = nullptr;
			}

			FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
		}
	}
}

bool ADialogueStandPositionTemplate::IsInEditorMode() const
{
	UWorld* World = GetWorld();
	return World && World->WorldType == EWorldType::Editor;
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
