// Fill out your copyright notice in the Description page of Project Settings.


#include "TwoTargetCameraTrackingSection.h"
#include "MovieSceneChannelProxy.h"
#include "CineCameraComponent.h"
#include "GameFramework/Character.h"
#include "DialogueCameraUtils.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AutoGenDialogueRuntimeSettings.h"

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

struct FTwoTargetCameraTrackingTrackData : IPersistentEvaluationData
{
	FVector FrontTargetLocation;
	FVector BackTargetLocation;
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

				EvaluateCameraPosition(Context, Operand, PersistentData, Player);
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

			void EvaluateCameraPosition(const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand& Operand, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const
			{
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

						float CameraYaw, FrontTargetRate, BackTargetRate;

						CameraTrackingSection->CameraYaw.Evaluate(Context.GetTime(), CameraYaw);
						if (CameraYaw == 0.f)
						{
							continue;
						}
						FTwoTargetCameraTrackingTrackData& TwoTargetCameraTrackingTrackData = PersistentData.GetOrAddTrackData<FTwoTargetCameraTrackingTrackData>();

						FVector FrontLocation, BackLocation;
						if (CameraTrackingSection->bKeepInitializeTargetLocation)
						{
							FrontLocation = TwoTargetCameraTrackingTrackData.FrontTargetLocation;
							BackLocation = TwoTargetCameraTrackingTrackData.BackTargetLocation;
						}
						else
						{
							FrontLocation = FrontTarget->GetPawnViewLocation();
							FrontLocation = BackTarget->GetPawnViewLocation();
						}

						FVector CameraLocation;
						FRotator CameraRotation;
						FVector FocusCenterLocation;
						CameraTrackingSection->FrontTargetRate.Evaluate(Context.GetTime(), FrontTargetRate);
						CameraTrackingSection->BackTargetRate.Evaluate(Context.GetTime(), BackTargetRate);
						FDialogueCameraUtils::CameraTrackingTwoTargets(CameraYaw, FMath::Clamp(FrontTargetRate, -1.f, 0.49999f), FMath::Clamp(BackTargetRate, -1.f, 0.49999f),
							FrontTarget->GetPawnViewLocation(), BackTarget->GetPawnViewLocation(), CineCameraComponent->FieldOfView, CameraLocation, CameraRotation, FocusCenterLocation);
						CineCameraComponent->SetWorldLocationAndRotation(CameraLocation, CameraRotation);

						// 迭代位置，防止摄像机穿过模型
						// 未实现
			// 			const int32 MaxIteratorCount = 10;
			// 			const float DeltaSeconds = Context.GetDelta() / Context.GetFrameRate();
			// 
			// 			const float IteratorDegree = 2.f * DeltaSeconds;
			// 			for (int32& Idx = TwoTargetCameraTrackingTrackData.CameraIteratorNum; Idx < MaxIteratorCount; ++Idx)
			// 			{
			// 				float CameraYawIteratored = CameraYaw + Idx * (CameraYaw > 0 ? IteratorDegree : -IteratorDegree);
			// 
			// 				CameraTrackingSection->FrontTargetRate.Evaluate(Context.GetTime(), FrontTargetRate);
			// 				CameraTrackingSection->BackTargetRate.Evaluate(Context.GetTime(), BackTargetRate);
			// 				FDialogueCameraUtils::CameraTrackingTwoTargets(CameraYawIteratored, FMath::Clamp(FrontTargetRate, -1.f, 0.49999f), FMath::Clamp(BackTargetRate, -1.f, 0.49999f),
			// 					FrontTarget->GetPawnViewLocation(), BackTarget->GetPawnViewLocation(), CineCameraComponent->FieldOfView, CameraLocation, CameraRotation, FocusCenterLocation);
			// 
			// 				FHitResult CameraHitResult;
			// 				UKismetSystemLibrary::BoxTraceSingle(FrontTarget, CameraLocation, FocusCenterLocation, FVector(0.1f, 20.f, 20.f), (FocusCenterLocation - CameraLocation).Rotation(), GetDefault<UAutoGenDialogueRuntimeSettings>()->CameraEvaluateTraceChannel, false, {}, EDrawDebugTrace::ForOneFrame, CameraHitResult, false);
			// 				float DistanceToCamera = FVector(CameraHitResult.ImpactPoint - CameraLocation).Size();
			// 				if (!CameraHitResult.bBlockingHit || DistanceToCamera > 100.f)
			// 				{
			// 					CineCameraComponent->SetWorldLocationAndRotation(CameraLocation, CameraRotation);
			// 					return;
			// 				}
			// 			}
			// 			
			// 			{
			// 				CineCameraComponent->SetWorldLocationAndRotation(CameraLocation, CameraRotation);
			// 			}
					}
				}
			}

			const UTwoTargetCameraTrackingSection* CameraTrackingSection;
		};

		if (CameraTrackingSection->bOnlyInitializeEvaluate == false)
		{
			ExecutionTokens.Add(FTwoTargetCameraTrackingSectionToken(CameraTrackingSection));
		}
	}
}

void FTwoTargetCameraTrackingSectionTemplate::TearDown(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const
{
	MOVIESCENE_DETAILED_SCOPE_CYCLE_COUNTER(MovieSceneEval_TwoTargetCameraTrackingSection_TearDown);

	// TODO: 恢复状态
}
