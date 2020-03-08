// Fill out your copyright notice in the Description page of Project Settings.


#include "Datas/DialogueCamera_OneTargetTracking.h"
#include <GameFramework/Character.h>
#include <Components/SkeletalMeshComponent.h>

#include "Tracks/CameraTrackingTrack/OneTargetCameraTrackingTrack.h"
#include "Tracks/CameraTrackingTrack/OneTargetCameraTrackingSection.h"
#include "Datas/GenDialogueSequenceConfigBase.h"


ADialogueCamera_OneTargetTracking::ADialogueCamera_OneTargetTracking()
{
	CameraTarget = CreateDefaultSubobject<UChildActorComponent>(GET_MEMBER_NAME_CHECKED(ADialogueCamera_OneTargetTracking, CameraTarget));
	{
		CameraTarget->SetupAttachment(TemplateRoot);
	}
}

void ADialogueCamera_OneTargetTracking::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (ACharacter* TargetCharacter = Cast<ACharacter>(CameraTarget->GetChildActor()))
	{
		TargetCharacter->SetActorRelativeLocation(FVector(0.f, 0.f, TargetCharacter->GetDefaultHalfHeight()));
		
		const FVector TargetLocation = GetActorTransform().InverseTransformPosition(TargetCharacter->GetMesh()->GetSocketLocation(SocketName) + TargetOffset);
		const FRotator LookAtRotation = (TargetLocation - CameraLocation).Rotation();
		CineCamera->SetRelativeLocation(CameraLocation);
		CineCamera->SetRelativeRotation(LookAtRotation);

		CameraRelativeLocation = CameraLocation - TargetLocation;
		CameraRelativeRotation = LookAtRotation;
	}
}

TOptional<AAutoGenDialogueCameraTemplate::FCameraWeightsData> ADialogueCamera_OneTargetTracking::EvaluateCameraTemplate(ACharacter* LookTarget, const TArray<ACharacter*>& Others, const TMap<ACharacter*, FGenDialogueCharacterData>& DialogueCharacterDataMap, float DialogueProgress) const
{
	if (Others.Num() > 0)
	{
		return TOptional<FCameraWeightsData>();
	}

	FCameraWeightsData CameraWeightsData;
	CameraWeightsData.CameraTemplate = this;
	FTransform TargetTransform = LookTarget->GetActorTransform();
	TargetTransform.SetLocation(LookTarget->GetMesh()->GetSocketLocation(SocketName) + TargetOffset);
	CameraWeightsData.CameraLocation = TargetTransform.TransformPosition(CameraRelativeLocation);
	CameraWeightsData.CameraRotation = TargetTransform.TransformRotation(CameraRelativeRotation.Quaternion()).Rotator();
	CameraWeightsData.Weights = 1.f;
	return CameraWeightsData;
}

void ADialogueCamera_OneTargetTracking::GenerateCameraTrackData(ACharacter* LookTarget, const TArray<ACharacter*>& Others, UMovieScene& MovieScene, FGuid CineCameraComponentGuid, const TMap<ACharacter*, FGenDialogueCharacterData>& DialogueCharacterDataMap, const TArray<FDialogueCameraCutData>& DialogueCameraCutDatas) const
{
	const FGenDialogueCharacterData& SpeakCharacterData = DialogueCharacterDataMap[LookTarget];
	UOneTargetCameraTrackingTrack* TwoTargetCameraTrackingTrack = MovieScene.AddTrack<UOneTargetCameraTrackingTrack>(CineCameraComponentGuid);
	UOneTargetCameraTrackingSection* TwoTargetCameraTrackingSection = TwoTargetCameraTrackingTrack->AddNewSentenceOnRow(SpeakCharacterData.BindingID);
	TwoTargetCameraTrackingSection->CreateFrom = GetClass();
	TwoTargetCameraTrackingSection->Offset = TargetOffset;
	TwoTargetCameraTrackingSection->SocketName = SocketName;
	TwoTargetCameraTrackingSection->RelativeLocation = CameraRelativeLocation;
}
