// Fill out your copyright notice in the Description page of Project Settings.


#include "TwoTargetCameraTrackingSection.h"
#include "MovieSceneChannelProxy.h"
#include "CineCameraComponent.h"
#include "GameFramework/Character.h"
#include "DialogueCameraUtils.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AutoGenDialogueRuntimeSettings.h"
#include "CanvasTypes.h"
#include "CanvasItem.h"
#include "Components/SkeletalMeshComponent.h"

DECLARE_CYCLE_STAT(TEXT("Two Target Camera Tracking Section Evaluate"), MovieSceneEval_TwoTargetCameraTrackingSection_Evaluate, STATGROUP_MovieSceneEval);
DECLARE_CYCLE_STAT(TEXT("Two Target Camera Tracking Section Tear Down"), MovieSceneEval_TwoTargetCameraTrackingSection_TearDown, STATGROUP_MovieSceneEval);
DECLARE_CYCLE_STAT(TEXT("Two Target Camera Tracking Section Token Execute"), MovieSceneEval_TwoTargetCameraTrackingSection_TokenExecute, STATGROUP_MovieSceneEval);

UTwoTargetCameraTrackingSection::UTwoTargetCameraTrackingSection()
{
	CameraYaw.SetDefault(30.f);
	FrontTargetRate.SetDefault(0.3f);
	BackTargetRate.SetDefault(0.4f);

	// Set up the channel proxy
	FMovieSceneChannelProxyData Channels;

#if WITH_EDITOR
	struct FCameraTrackingChannelEditorData
	{
		FCameraTrackingChannelEditorData()
		{
			Data[0].SetIdentifiers("CameraYaw", NSLOCTEXT("TwoTargetCameraTrackingTrack", "CameraYaw", "相机偏航角"));
			Data[1].SetIdentifiers("FrontTargetRate", NSLOCTEXT("TwoTargetCameraTrackingTrack", "FrontTargetRate", "前景目标占比"));
			Data[2].SetIdentifiers("BackTargetRate", NSLOCTEXT("TwoTargetCameraTrackingTrack", "BackTargetRate", "背景目标占比"));
		}
		FMovieSceneChannelMetaData Data[3];
	};
	static const FCameraTrackingChannelEditorData EditorData;

	Channels.Add(CameraYaw, EditorData.Data[0], TMovieSceneExternalValue<float>());
	Channels.Add(FrontTargetRate, EditorData.Data[1], TMovieSceneExternalValue<float>());
	Channels.Add(BackTargetRate, EditorData.Data[2], TMovieSceneExternalValue<float>());
#else

	Channels.Add(CameraYaw);
	Channels.Add(FrontTargetRate);
	Channels.Add(BackTargetRate);
#endif

	ChannelProxy = MakeShared<FMovieSceneChannelProxy>(MoveTemp(Channels));
}

TOptional<TRange<FFrameNumber>> UTwoTargetCameraTrackingSection::GetAutoSizeRange() const
{
	return Super::GetAutoSizeRange();
}

void UTwoTargetCameraTrackingSection::TrimSection(FQualifiedFrameTime TrimTime, bool bTrimLeft)
{
	SetFlags(RF_Transactional);

	Super::TrimSection(TrimTime, bTrimLeft);
}

UMovieSceneSection* UTwoTargetCameraTrackingSection::SplitSection(FQualifiedFrameTime SplitTime)
{
	return Super::SplitSection(SplitTime);
}

TOptional<FFrameTime> UTwoTargetCameraTrackingSection::GetOffsetTime() const
{
	return TOptional<FFrameTime>();
}

FMovieSceneEvalTemplatePtr UTwoTargetCameraTrackingSection::GenerateTemplate() const
{
	return FTwoTargetCameraTrackingSectionTemplate(*this);
}

#if WITH_EDITOR
void UTwoTargetCameraTrackingSection::DrawSectionSelectedPreviewInfo(IMovieScenePlayer* Player, const FFrameNumber& FramePosition, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) const
{

}

void UTwoTargetCameraTrackingSection::DrawSectionExecutePreviewInfo(IMovieScenePlayer* Player, const FFrameNumber& FramePosition, FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) const
{
	for (const TWeakObjectPtr<>& Object : Player->FindBoundObjects(BackTarget.GetGuid(), BackTarget.GetSequenceID()))
	{
		if (ACharacter* BackTargetRef = Cast<ACharacter>(Object.Get()))
		{
			FVector Origin, BoxExtent;
			BackTargetRef->GetActorBounds(true, Origin, BoxExtent);

			const FIntRect CanvasRect = Canvas->GetViewRect();

			{
				float CameraYawValue;
				CameraYaw.Evaluate(FramePosition, CameraYawValue);
				const bool FrontIsRight = CameraYawValue > 0.f;

				float BackTargetRateValue;
				BackTargetRate.Evaluate(FramePosition, BackTargetRateValue);
				BackTargetRateValue = FMath::Clamp(BackTargetRateValue, -1.f, 0.49999f);
				const float BackTargetRateX = (FrontIsRight ? BackTargetRateValue : 1.f - BackTargetRateValue);
				{
					const float BackTargetX = CanvasRect.Width() * (BackTargetRateX - BackTolerance);
					FCanvasLineItem CanvasLineItem(FVector2D(BackTargetX, CanvasRect.Min.Y), FVector2D(BackTargetX, CanvasRect.Max.Y));
					CanvasLineItem.LineThickness = 1.f;
					CanvasLineItem.SetColor(DebugColor);
					Canvas->DrawItem(CanvasLineItem);
				}
				{
					const float BackTargetX = CanvasRect.Width() * (BackTargetRateX + BackTolerance);
					FCanvasLineItem CanvasLineItem(FVector2D(BackTargetX, CanvasRect.Min.Y), FVector2D(BackTargetX, CanvasRect.Max.Y));
					CanvasLineItem.LineThickness = 1.f;
					CanvasLineItem.SetColor(DebugColor);
					Canvas->DrawItem(CanvasLineItem);
				}

				{
					const FVector BackLocation = BackTargetTrackingSocketName.IsNone() ? BackTargetRef->GetPawnViewLocation() : BackTargetRef->GetMesh()->GetSocketLocation(BackTargetTrackingSocketName);
					FVector2D ScreenPosition;
					FDialogueCameraUtils::ProjectWorldToScreen(View, CanvasRect, BackLocation, ScreenPosition);
					const float ScreenRaduis = FDialogueCameraUtils::ConvertWorldSphereRadiusToScreen(View, BackLocation, BackTargetVolumnRadius) * CanvasRect.Width();
					FCanvasLineItem CanvasLineItem(FVector2D(ScreenPosition.X - ScreenRaduis, ScreenPosition.Y), FVector2D(ScreenPosition.X + ScreenRaduis, ScreenPosition.Y));
					CanvasLineItem.LineThickness = 1.f;
					CanvasLineItem.SetColor(DebugColor);
					Canvas->DrawItem(CanvasLineItem);
				}
			}
		}
	}
}
#endif

struct FTwoTargetCameraTrackingSectionData : IPersistentEvaluationData
{
	// 用于确定是否是连续的镜头
	FFrameNumber PrevFrameNumber = -2;
};

void FTwoTargetCameraTrackingSectionTemplate::Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const
{
	MOVIESCENE_DETAILED_SCOPE_CYCLE_COUNTER(MovieSceneEval_TwoTargetCameraTrackingSection_Evaluate);

	if (Context.GetStatus() != EMovieScenePlayerStatus::Jumping)
	{
		struct FTwoTargetCameraTrackingSectionToken : IMovieSceneExecutionToken
		{
			FTwoTargetCameraTrackingSectionToken(const UTwoTargetCameraTrackingSection* InCameraTrackingSection)
				:CameraTrackingSection(InCameraTrackingSection)
			{}

			void Execute(const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand& Operand, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) override
			{
				MOVIESCENE_DETAILED_SCOPE_CYCLE_COUNTER(MovieSceneEval_TwoTargetCameraTrackingSection_TokenExecute);
				if (Operand.ObjectBindingID.IsValid())
				{
					for (const TWeakObjectPtr<>& CameraObject : Player.FindBoundObjects(Operand))
					{
						UCineCameraComponent* CineCameraComponent = Cast<UCineCameraComponent>(CameraObject.Get());
						if (!CineCameraComponent)
						{
							continue;
						}

						ACharacter* FrontTarget = TryGetFrontTarget(Player);
						ACharacter* BackTarget = TryGetBackTarget(Player);

						if (!FrontTarget || !BackTarget)
						{
							continue;
						}

						FTwoTargetCameraTrackingSectionData& TwoTargetCameraTrackingSectionData = PersistentData.GetOrAddSectionData<FTwoTargetCameraTrackingSectionData>();
						const TRange<FFrameNumber> FrameNumberRange = Context.GetFrameNumberRange();
						const bool IsFirstEvaluate = FrameNumberRange.GetLowerBoundValue() != (TwoTargetCameraTrackingSectionData.PrevFrameNumber + 1);
						TwoTargetCameraTrackingSectionData.PrevFrameNumber = FrameNumberRange.GetUpperBoundValue();

						float CameraYaw, FrontTargetRate, BackTargetRate;
						CameraTrackingSection->CameraYaw.Evaluate(Context.GetTime(), CameraYaw);
						if (CameraYaw == 0.f)
						{
							continue;
						}
						CameraTrackingSection->FrontTargetRate.Evaluate(Context.GetTime(), FrontTargetRate);
						CameraTrackingSection->BackTargetRate.Evaluate(Context.GetTime(), BackTargetRate);

						const FVector FrontLocation = FrontTarget->GetPawnViewLocation();
						const FVector BackLocation = CameraTrackingSection->BackTargetTrackingSocketName.IsNone() ? BackTarget->GetPawnViewLocation() : BackTarget->GetMesh()->GetSocketLocation(CameraTrackingSection->BackTargetTrackingSocketName);

						FMinimalViewInfo MinimalViewInfo;
						CineCameraComponent->GetCameraView(0.f, MinimalViewInfo);
						FVector2D BackScreenPosition;
						FDialogueCameraUtils::ProjectWorldToScreen(MinimalViewInfo, BackLocation, BackScreenPosition);
						const float ScreenRaduis = FDialogueCameraUtils::ConvertWorldSphereRadiusToScreen(CineCameraComponent, BackLocation, CameraTrackingSection->BackTargetVolumnRadius);

						const bool FrontIsRight = CameraYaw > 0.f;
						if (!FrontIsRight)
						{
							BackScreenPosition.X = 1.f - BackScreenPosition.X;
						}

						if (CameraTrackingSection->BackTolerance == 0.f)
						{
							if (BackScreenPosition.X == BackTargetRate)
							{
								return;
							}
						}
						else
						{
							float LeftLimit = BackTargetRate - CameraTrackingSection->BackTolerance;
							float RightLimit = BackTargetRate + CameraTrackingSection->BackTolerance;

							const bool IsValid = RightLimit - LeftLimit > ScreenRaduis * 2.f;
							if (IsValid)
							{
								LeftLimit += ScreenRaduis;
								RightLimit -= ScreenRaduis;

								if (BackScreenPosition.X > RightLimit)
								{
									BackTargetRate = RightLimit;
								}
								else if (BackScreenPosition.X < LeftLimit)
								{
									BackTargetRate = LeftLimit;
								}
								else
								{
									return;
								}
							}
						}

						FVector CameraLocation;
						FRotator CameraRotation;
						FVector FocusCenterLocation;
						FDialogueCameraUtils::CameraTrackingTwoTargets(CameraYaw, FMath::Clamp(FrontTargetRate, -1.f, 0.49999f), FMath::Clamp(BackTargetRate, -1.f, 0.49999f),
							FrontLocation + CameraTrackingSection->FrontOffset, BackLocation + CameraTrackingSection->BackOffset, CineCameraComponent->FieldOfView, CameraLocation, CameraRotation, FocusCenterLocation);

						if (IsFirstEvaluate)
						{
							CineCameraComponent->SetWorldLocationAndRotation(CameraLocation, CameraRotation);
						}
						else
						{
							const FVector PrevLocation = CineCameraComponent->GetComponentLocation();
							const FRotator PrevRotation = CineCameraComponent->GetComponentRotation();

							const float DeltaTime = Context.GetDelta() / Context.GetFrameRate();
							CineCameraComponent->SetWorldLocationAndRotation(FMath::VInterpTo(PrevLocation, CameraLocation, DeltaTime, 2.f), FMath::RInterpTo(PrevRotation, CameraRotation, DeltaTime, 2.f));
						}
					}
				}
			}

			ACharacter* TryGetFrontTarget(IMovieScenePlayer& Player) const
			{
				for (const TWeakObjectPtr<>& Object : Player.FindBoundObjects(CameraTrackingSection->FrontTarget.GetGuid(), CameraTrackingSection->FrontTarget.GetSequenceID()))
				{
					if (ACharacter* FrontTarget = Cast<ACharacter>(Object.Get()))
					{
						return FrontTarget;
					}
				}
				return nullptr;
			}
			ACharacter* TryGetBackTarget(IMovieScenePlayer& Player) const
			{
				for (const TWeakObjectPtr<>& Object : Player.FindBoundObjects(CameraTrackingSection->BackTarget.GetGuid(), CameraTrackingSection->BackTarget.GetSequenceID()))
				{
					if (ACharacter* BackTarget = Cast<ACharacter>(Object.Get()))
					{
						return BackTarget;
					}
				}
				return nullptr;
			}

			const UTwoTargetCameraTrackingSection* CameraTrackingSection;
		};

		ExecutionTokens.Add(FTwoTargetCameraTrackingSectionToken(CameraTrackingSection));
	}
}

void FTwoTargetCameraTrackingSectionTemplate::TearDown(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const
{
	MOVIESCENE_DETAILED_SCOPE_CYCLE_COUNTER(MovieSceneEval_TwoTargetCameraTrackingSection_TearDown);

	// TODO: 恢复状态
}
