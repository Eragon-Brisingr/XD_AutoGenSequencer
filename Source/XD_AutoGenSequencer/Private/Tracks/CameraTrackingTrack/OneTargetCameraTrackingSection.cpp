// Fill out your copyright notice in the Description page of Project Settings.

#include "Tracks/CameraTrackingTrack/OneTargetCameraTrackingSection.h"
#include <CineCameraComponent.h>
#include <GameFramework/Character.h>
#include <Components/SkeletalMeshComponent.h>

FMovieSceneEvalTemplatePtr UOneTargetCameraTrackingSection::GenerateTemplate() const
{
	return FOneTargetCameraTrackingSectionTemplate(*this);
}

void FOneTargetCameraTrackingSectionTemplate::Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const
{
	if (Context.GetStatus() != EMovieScenePlayerStatus::Jumping)
	{
		struct FOneTargetCameraTrackingSectionToken : IMovieSceneExecutionToken
		{
			FOneTargetCameraTrackingSectionToken(const UOneTargetCameraTrackingSection* InCameraTrackingSection)
				:CameraTrackingSection(InCameraTrackingSection)
			{}

			void Execute(const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand& Operand, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) override
			{
				if (Operand.ObjectBindingID.IsValid())
				{
					struct FOneTargetCameraTrackingSectionData : IPersistentEvaluationData
					{
						// 用于确定是否是连续的镜头
						FFrameNumber PrevFrameNumber = -2;
					};

					FOneTargetCameraTrackingSectionData& OneTargetCameraTrackingSectionData = PersistentData.GetOrAddSectionData<FOneTargetCameraTrackingSectionData>();
					const TRange<FFrameNumber> FrameNumberRange = Context.GetFrameNumberRange();
					const bool IsFirstEvaluate = FrameNumberRange.GetLowerBoundValue() != (OneTargetCameraTrackingSectionData.PrevFrameNumber + 1);
					OneTargetCameraTrackingSectionData.PrevFrameNumber = FrameNumberRange.GetUpperBoundValue();

					for (const TWeakObjectPtr<>& CameraObject : Player.FindBoundObjects(Operand))
					{
						UCineCameraComponent* CineCameraComponent = Cast<UCineCameraComponent>(CameraObject.Get());
						if (!CineCameraComponent)
						{
							continue;
						}

						ACharacter* CameraTarget = nullptr;
						for (const TWeakObjectPtr<>& Object : Player.FindBoundObjects(CameraTrackingSection->Target.GetGuid(), CameraTrackingSection->Target.GetSequenceID()))
						{
							CameraTarget = Cast<ACharacter>(Object.Get());
							if (CameraTarget)
							{
								break;
							}
						}

						if (CameraTarget == nullptr)
						{
							return;
						}
						
						const FTransform TargetTransform = CameraTarget->GetActorTransform();
						const FVector SocketLocation = CameraTarget->GetMesh()->GetSocketLocation(CameraTrackingSection->SocketName);
						if (IsFirstEvaluate)
						{
							CineCameraComponent->SetWorldLocation(SocketLocation + TargetTransform.TransformVector(CameraTrackingSection->RelativeLocation));
						}
						const FRotator LookAtRotation = (SocketLocation + TargetTransform.TransformVector(CameraTrackingSection->Offset) - CineCameraComponent->GetComponentLocation()).Rotation();
						CineCameraComponent->SetWorldRotation(LookAtRotation);
					}
				}
			}

			const UOneTargetCameraTrackingSection* CameraTrackingSection;
		};

		ExecutionTokens.Add(FOneTargetCameraTrackingSectionToken(CameraTrackingSection));
	}
}

void FOneTargetCameraTrackingSectionTemplate::TearDown(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const
{

}
