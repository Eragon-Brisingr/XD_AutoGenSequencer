// Fill out your copyright notice in the Description page of Project Settings.


#include "Datas/DialogueCamera_TwoTargetTracking.h"
#include <CineCameraActor.h>
#include <Editor.h>
#include <Kismet/KismetSystemLibrary.h>
#include <GameFramework/Character.h>
#include <CineCameraComponent.h>

#include "Datas/DialogueStandPositionTemplate.h"
#include "Datas/GenDialogueSequenceConfigBase.h"
#include "Tracks/CameraTrackingTrack/TwoTargetCameraTrackingTrack.h"
#include "Tracks/CameraTrackingTrack/TwoTargetCameraTrackingSection.h"
#include "Utils/DialogueCameraUtils.h"
#include "Utils/AutoGenDialogueRuntimeSettings.h"
#include "Components/TextRenderComponent.h"
#include "Utils/AutoGenSequence_Log.h"

#define LOCTEXT_NAMESPACE "FXD_AutoGenSequencer_EditorModule"

ADialogueCamera_TwoTargetTracking::ADialogueCamera_TwoTargetTracking()
{
	FrontCharacterComponent = CreateDefaultSubobject<UChildActorComponent>(GET_MEMBER_NAME_CHECKED(ADialogueCamera_TwoTargetTracking, FrontCharacterComponent));
	{
		FrontCharacterComponent->SetupAttachment(TemplateRoot);
		FrontCharacterComponent->SetWorldLocationAndRotation(FVector(-100.f, 0.f, 88.f), FRotator(0.f, 0.f, 0.f));
		FrontCharacterComponent->SetChildActorClass(ACharacter::StaticClass());
	}
	PreviewFrontHint = CreateDefaultSubobject<UTextRenderComponent>(GET_MEMBER_NAME_CHECKED(ADialogueCamera_TwoTargetTracking, PreviewFrontHint), true);
	{
		PreviewFrontHint->SetText(LOCTEXT("Front Target Hint", "Front"));
		PreviewFrontHint->HorizontalAlignment = EHorizTextAligment::EHTA_Center;
		PreviewFrontHint->SetupAttachment(FrontCharacterComponent);
	}
	BackCharacterComponent = CreateDefaultSubobject<UChildActorComponent>(GET_MEMBER_NAME_CHECKED(ADialogueCamera_TwoTargetTracking, BackCharacterComponent));
	{
		BackCharacterComponent->SetupAttachment(TemplateRoot);
		BackCharacterComponent->SetWorldLocationAndRotation(FVector(100.f, 0.f, 88.f), FRotator(0.f, 180.f, 0.f));
		BackCharacterComponent->SetChildActorClass(ACharacter::StaticClass());
	}
	PreviewBackHint = CreateDefaultSubobject<UTextRenderComponent>(GET_MEMBER_NAME_CHECKED(ADialogueCamera_TwoTargetTracking, PreviewBackHint), true);
	{
		PreviewBackHint->SetText(LOCTEXT("Back Target Hint", "Back"));
		PreviewBackHint->HorizontalAlignment = EHorizTextAligment::EHTA_Center;
		PreviewBackHint->SetupAttachment(BackCharacterComponent);
	}
}

void ADialogueCamera_TwoTargetTracking::UpdateCameraTransform()
{
	ACharacter* FrontCharacter = Cast<ACharacter>(FrontCharacterComponent->GetChildActor());
	ACharacter* BackCharacter = Cast<ACharacter>(BackCharacterComponent->GetChildActor());

	if (FrontCharacter && BackCharacter)
	{
		const FVector FrontPos = FrontCharacter->GetPawnViewLocation();
		const FVector BackPos = BackCharacter->GetPawnViewLocation();

		FVector CameraLocation;
		FRotator CameraRotation;
		FVector FocusCenterLocation;
		FDialogueCameraUtils::CameraTrackingTwoTargets(CameraYawAngle, FrontTargetRate, BackTargetRate, FrontPos + FrontOffset, BackPos + BackOffset,
			CineCameraComponent->CurrentHorizontalFOV, CameraLocation, CameraRotation, FocusCenterLocation);

		CineCameraComponent->SetWorldLocationAndRotation(CameraLocation, CameraRotation);

		const FVector HintOffset = FVector(0.f, 0.f, 50.f);
		PreviewFrontHint->SetVisibility(true);
		PreviewBackHint->SetVisibility(true);
		PreviewFrontHint->SetWorldLocationAndRotation(FrontPos + HintOffset, FrontCharacter->GetActorRotation());
		PreviewBackHint->SetWorldLocationAndRotation(BackPos + HintOffset, BackCharacter->GetActorRotation());
	}
	else
	{
		PreviewFrontHint->SetVisibility(false);
		PreviewBackHint->SetVisibility(false);
	}
}

TOptional<AAutoGenDialogueCameraTemplate::FCameraWeightsData> ADialogueCamera_TwoTargetTracking::EvaluateCameraTemplate(ACharacter* LookTarget, const TArray<ACharacter*>& Others, const TMap<ACharacter*, FGenDialogueCharacterData>& DialogueCharacterDataMap, float DialogueProgress) const
{
	if (Others.Num() == 0)
	{
		return TOptional<FCameraWeightsData>();
	}

	FCameraWeightsData CameraWeightsData;
	ACharacter* Target = Others[0];
	CameraWeightsData.CameraTemplate = this;

	FVector SpeakerLocation = LookTarget->GetPawnViewLocation();
	FVector TargetLocation = Target->GetPawnViewLocation();

	const FGenDialogueCharacterData& SpeakCharacterData = DialogueCharacterDataMap[LookTarget];
	const FGenDialogueCharacterData& TargetCharacterData = DialogueCharacterDataMap[Target];

	// 防止越轴的处理
	bool InvertCameraPos = SpeakCharacterData.CharacterIdx - TargetCharacterData.CharacterIdx > 0;
	const float CameraYaw = !InvertCameraPos ? CameraYawAngle : -CameraYawAngle;

	FVector CameraLocation;
	FRotator CameraRotation;
	FVector FocusCenterLocation;
	FDialogueCameraUtils::CameraTrackingTwoTargets(CameraYaw, FrontTargetRate, BackTargetRate, SpeakerLocation + FrontOffset, TargetLocation + BackOffset, CineCameraComponent->FieldOfView, CameraLocation, CameraRotation, FocusCenterLocation);

	CameraWeightsData.CameraLocation = CameraLocation;
	CameraWeightsData.CameraRotation = CameraRotation;

	// TODO：这个估值有问题，需要调整
	float DistanceDialogueProgressWeights = FMath::Abs(0.5f - DialogueProgress) * 2.f;
	float CameraDistance = (CameraLocation - FocusCenterLocation).Size();
	float CharacterDistance = (SpeakerLocation - TargetLocation).Size();
	float DistanceWeights = FMath::Clamp(CameraDistance / (CharacterDistance * 2.f), 0.f, 1.f);

	CameraWeightsData.Weights = 1.f - FMath::Abs(DistanceWeights - DistanceDialogueProgressWeights);

	// TODO：避免穿入碰撞内部
	// TODO：考虑覆盖屏幕百分比
// 		FHitResult CameraHitResult;
// 		if (UKismetSystemLibrary::BoxTraceSingle(GEditor->GetEditorWorldContext().World(), CameraLocation, FocusCenterLocation, FVector(0.1f, 20.f, 20.f), (FocusCenterLocation - CameraLocation).Rotation(), GetDefault<UAutoGenDialogueRuntimeSettings>()->CameraEvaluateTraceChannel, false, {}, EDrawDebugTrace::None, CameraHitResult, false))
// 		{
// 			float DistanceToCameraLocation = (CameraLocation - CameraHitResult.ImpactPoint).Size();
// 			CameraWeightsData.Weights *= DistanceToCameraLocation / CameraDistance;
// 		}
	return TOptional<FCameraWeightsData>(CameraWeightsData);
}

void ADialogueCamera_TwoTargetTracking::GenerateCameraTrackData(ACharacter* LookTarget, const TArray<ACharacter*>& Others, UMovieScene& MovieScene, FGuid CineCameraComponentGuid, const TMap<ACharacter*, FGenDialogueCharacterData>& DialogueCharacterDataMap, const TArray<FDialogueCameraCutData>& DialogueCameraCutDatas) const
{
	check(Others.Num() > 0);
	ACharacter* Target = Others[0];

	const FGenDialogueCharacterData& SpeakCharacterData = DialogueCharacterDataMap[LookTarget];
	const FGenDialogueCharacterData& TargetCharacterData = DialogueCharacterDataMap[Target];

	// 防止越轴的处理
	bool InvertCameraPos = SpeakCharacterData.CharacterIdx - TargetCharacterData.CharacterIdx > 0;
	const float CameraYaw = !InvertCameraPos ? CameraYawAngle : -CameraYawAngle;
	UTwoTargetCameraTrackingTrack* TwoTargetCameraTrackingTrack = MovieScene.AddTrack<UTwoTargetCameraTrackingTrack>(CineCameraComponentGuid);
	UTwoTargetCameraTrackingSection* TwoTargetCameraTrackingSection = TwoTargetCameraTrackingTrack->AddNewSentenceOnRow(TargetCharacterData.BindingID, FrontOffset, SpeakCharacterData.BindingID, BackOffset);
	TwoTargetCameraTrackingSection->CreateFrom = GetClass();
	TwoTargetCameraTrackingSection->CameraYaw.SetDefault(CameraYaw);
	TwoTargetCameraTrackingSection->FrontTargetRate.SetDefault(FrontTargetRate);
	TwoTargetCameraTrackingSection->BackTargetRate.SetDefault(BackTargetRate);
	
	// TODO：运镜，镜头动画？
// 	for (const FDialogueCameraCutData& DialogueCameraCutData : DialogueCameraCutDatas)
// 	{
// 		const TRange<FFrameNumber> CameraCutRange = DialogueCameraCutData.CameraCutRange;
// 
// 		TwoTargetCameraTrackingSection->CameraYaw.AddLinearKey(CameraCutRange.GetLowerBoundValue(), CameraYaw);
// 		TwoTargetCameraTrackingSection->CameraYaw.AddLinearKey(CameraCutRange.GetUpperBoundValue(), CameraYaw + (CameraYaw > 0 ? 10.f : -10.f));
// 	}
}

#undef LOCTEXT_NAMESPACE
