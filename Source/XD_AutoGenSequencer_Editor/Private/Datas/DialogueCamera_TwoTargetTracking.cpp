// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueCamera_TwoTargetTracking.h"
#include "TwoTargetCameraTrackingTrack.h"
#include "TwoTargetCameraTrackingSection.h"
#include "DialogueCameraUtils.h"
#include "CineCameraActor.h"
#include "DialogueStandPositionTemplate.h"
#include "GameFramework/Character.h"
#include "CineCameraComponent.h"

void ADialogueCamera_TwoTargetTracking::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ACineCameraActor* CineCameraActor = CastChecked<ACineCameraActor>(CineCamera->GetChildActorTemplate());
	CineCameraComponent = CineCameraActor->GetCineCameraComponent();

	if (const ADialogueStandPositionTemplate * StandPositionTemplate = Cast<ADialogueStandPositionTemplate>(StandTemplatePreview->GetChildActor()))
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
				FVector FocusCenterLocation;
				FDialogueCameraUtils::CameraTrackingTwoTargets(CameraYawAngle, FrontTargetRate, BackTargetRate, FrontPos, BackPos,
					CineCameraComponent->CurrentHorizontalFOV, CameraLocation, CameraRotation, FocusCenterLocation);

				CineCamera->SetWorldLocationAndRotation(CameraLocation, CameraRotation);
			}
		}
	}
}

AAutoGenDialogueCameraTemplate::FCameraWeightsData ADialogueCamera_TwoTargetTracking::EvaluateCameraTemplate(ACharacter* Speaker, const TArray<ACharacter*>& Targets, float DialogueProgress) const
{
	FCameraWeightsData CameraWeightsData;
	if (Targets.Num() > 0)
	{
		ACharacter* Target = Targets[0];
		CameraWeightsData.CameraTemplate = this;

		FVector SpeakerLocation = Speaker->GetPawnViewLocation();
		FVector TargetLocation = Target->GetPawnViewLocation();

		FVector CameraLocation;
		FRotator CameraRotation;
		FVector FocusCenterLocation;
		FDialogueCameraUtils::CameraTrackingTwoTargets(CameraYawAngle, FrontTargetRate, BackTargetRate, SpeakerLocation, TargetLocation, CineCameraComponent->FieldOfView, CameraLocation, CameraRotation, FocusCenterLocation);

		CameraWeightsData.CameraLocation = CameraLocation;
		CameraWeightsData.CameraRotation = CameraRotation;

		// TODO：这个估值有问题，需要调整
		float DistanceDialogueProgressWeights = FMath::Abs(0.5f - DialogueProgress) * 2.f;
		float CameraDistance = (CameraLocation - FocusCenterLocation).Size();
		float CharacterDistance = (SpeakerLocation - TargetLocation).Size();
		float DistanceWeights = FMath::Clamp(CameraDistance / (CharacterDistance * 2.f), 0.f, 1.f);

		CameraWeightsData.Weights = 1.f - FMath::Abs(DistanceWeights - DistanceDialogueProgressWeights);
	}
	return CameraWeightsData;
}

void ADialogueCamera_TwoTargetTracking::GenerateCameraTrackData(ACharacter* Speaker, const TArray<ACharacter*>& Targets, UMovieScene& MovieScene, FGuid CineCameraComponentGuid, const TMap<ACharacter*, FMovieSceneObjectBindingID>& InstanceBindingIdMap, const TMap<ACharacter*, int32>& InstanceIdxMap) const
{
	check(Targets.Num() > 0);
	ACharacter* Target = Targets[0];

	bool InvertCameraPos = InstanceIdxMap[Speaker] - InstanceIdxMap[Target] > 0;
	UTwoTargetCameraTrackingTrack* TwoTargetCameraTrackingTrack = MovieScene.AddTrack<UTwoTargetCameraTrackingTrack>(CineCameraComponentGuid);
	UTwoTargetCameraTrackingSection* TwoTargetCameraTrackingSection = TwoTargetCameraTrackingTrack->AddNewSentenceOnRow(InstanceBindingIdMap[Target], InstanceBindingIdMap[Speaker]);
	TwoTargetCameraTrackingSection->CameraYaw.SetDefault(!InvertCameraPos ? CameraYawAngle : -CameraYawAngle);
	TwoTargetCameraTrackingSection->FrontTargetRate.SetDefault(FrontTargetRate);
	TwoTargetCameraTrackingSection->BackTargetRate.SetDefault(BackTargetRate);
}
