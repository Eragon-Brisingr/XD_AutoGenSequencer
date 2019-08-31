// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoGenDialogueCameraTemplate.h"
#include "DialogueStandPositionTemplate.h"
#include "Components/ChildActorComponent.h"
#include "CineCameraComponent.h"
#include "DialogueCameraUtils.h"
#include "GameFramework/Character.h"
#include "CineCameraActor.h"

// Sets default values
AAutoGenDialogueCameraTemplate::AAutoGenDialogueCameraTemplate()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

#if WITH_EDITOR
	StandTemplatePreview = CreateDefaultSubobject<UChildActorComponent>(GET_MEMBER_NAME_CHECKED(AAutoGenDialogueCameraTemplate, StandTemplatePreview));
	StandTemplatePreview->bIsEditorOnly = true;
	SetRootComponent(StandTemplatePreview);

	CineCamera = CreateDefaultSubobject<UChildActorComponent>(GET_MEMBER_NAME_CHECKED(AAutoGenDialogueCameraTemplate, CineCamera));
	CineCamera->bIsEditorOnly = true;
	CineCamera->SetChildActorClass(ACineCameraActor::StaticClass());
	CineCamera->SetupAttachment(StandTemplatePreview);
#endif
}

// Called when the game starts or when spawned
void AAutoGenDialogueCameraTemplate::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAutoGenDialogueCameraTemplate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAutoGenDialogueCameraTemplate::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (ACineCameraActor* CineCameraActor = Cast<ACineCameraActor>(CineCamera->GetChildActorTemplate()))
	{
		CineCameraComponent = CineCameraActor->GetCineCameraComponent();
	}

	if (const ADialogueStandPositionTemplate* StandPositionTemplate = Cast<ADialogueStandPositionTemplate>(StandTemplatePreview->GetChildActor()))
	{
		if (CineCameraComponent && StandPositionTemplate->PreviewCharacters.Num() >= 2)
		{
			const TArray<UChildActorComponent*>& PreviewCharacters = StandPositionTemplate->PreviewCharacters;
			ACharacter* FrontCharacter = Cast<ACharacter>(PreviewCharacters[0]->GetChildActor());
			ACharacter* BackCharacter = Cast<ACharacter>(PreviewCharacters[1]->GetChildActor());
			if (FrontCharacter && BackCharacter)
			{
				const FVector FrontPos = FrontCharacter->GetPawnViewLocation();
				const FVector BackPos = BackCharacter->GetPawnViewLocation();

				FVector CameraLocation;
				FRotator CameraRotation;
				FDialogueCameraUtils::CameraTrackingTwoTargets(CameraYawAngle, FrontTargetRate, BackTargetRate, FrontPos, BackPos, CineCameraComponent->CurrentHorizontalFOV, CineCameraComponent->AspectRatio, CameraLocation, CameraRotation);

				CineCamera->SetWorldLocationAndRotation(CameraLocation, CameraRotation);
			}
		}
	}
}

#if WITH_EDITOR

void AAutoGenDialogueCameraTemplate::PreEditChange(UProperty* PropertyThatWillChange)
{
	//Super::PreEditChange(PropertyThatWillChange);
}

void AAutoGenDialogueCameraTemplate::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	//ReregisterComponentsWhenModified()
	//Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AAutoGenDialogueCameraTemplate, StandTemplate))
	{
		StandTemplatePreview->SetChildActorClass(StandTemplate);
	}

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		RerunConstructionScripts();
	}
}
#endif

