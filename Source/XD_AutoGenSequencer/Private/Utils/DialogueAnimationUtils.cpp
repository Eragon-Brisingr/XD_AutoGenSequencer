// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/DialogueAnimationUtils.h"
#include <Animation/AnimSequence.h>
#include <BonePose.h>
#include <Animation/AnimCurveTypes.h>
#include <Animation/AnimationAsset.h>
#include <Animation/Skeleton.h>
#include <AnimationRuntime.h>

float UDialogueAnimationUtils::GetSimilarityWeights(const UAnimSequence* LHS, float LTime, const UAnimSequence* RHS, float RTime)
{
	check(LHS->GetSkeleton() == RHS->GetSkeleton());

	USkeleton& Skeletion = *LHS->GetSkeleton();
	const FReferenceSkeleton& ReferenceSkeleton = Skeletion.GetReferenceSkeleton();

	static const TArray<FName> BoneNames = { TEXT("head"), TEXT("hand_l"), TEXT("hand_r") };

	TArray<FBoneIndexType> RequiredBones;
	TArray<FBoneIndexType> RequiredBonesWithParents;
	{
		for (const FName& BoneName : BoneNames)
		{
			int32 RequiredBoneIdx = ReferenceSkeleton.FindBoneIndex(BoneName);
			if (RequiredBoneIdx != INDEX_NONE)
			{
				RequiredBones.Add(RequiredBoneIdx);
				for (int32 ParentBoneIdx = RequiredBoneIdx; ParentBoneIdx != INDEX_NONE && !RequiredBonesWithParents.Contains(ParentBoneIdx); ParentBoneIdx = ReferenceSkeleton.GetParentIndex(ParentBoneIdx))
				{
				 	RequiredBonesWithParents.Add(ParentBoneIdx);
				}
			}
		}
		RequiredBonesWithParents.Sort();
	}
	FBoneContainer BoneContainer(RequiredBonesWithParents, FCurveEvaluationOption(false), Skeletion);

	FCompactPose LPose;
	FBlendedCurve LCurve;
	{
		LPose.SetBoneContainer(&BoneContainer);
		FAnimExtractContext LAnimExtractContext;
		LAnimExtractContext.CurrentTime = LTime;
		LAnimExtractContext.bExtractRootMotion = false;
		LHS->GetAnimationPose(LPose, LCurve, LAnimExtractContext);
	}
	FCSPose<FCompactPose> LCSPose;
	LCSPose.InitPose(LPose);

	FCompactPose RPose;
	FBlendedCurve RCurve;
	{
		RPose.SetBoneContainer(&BoneContainer);
		FAnimExtractContext RAnimExtractContext;
		RAnimExtractContext.CurrentTime = RTime;
		RAnimExtractContext.bExtractRootMotion = false;
		RHS->GetAnimationPose(RPose, RCurve, RAnimExtractContext);
	}
	FCSPose<FCompactPose> RCSPose;
	RCSPose.InitPose(RPose);

	float DistanceCount = 0;
	for (FBoneIndexType BoneIndex : RequiredBones)
	{
		FCompactPoseBoneIndex CompactPoseBoneIndex = BoneContainer.GetCompactPoseIndexFromSkeletonIndex(BoneIndex);
		FTransform LTransform = LCSPose.GetComponentSpaceTransform(CompactPoseBoneIndex);
		FTransform RTransform = RCSPose.GetComponentSpaceTransform(CompactPoseBoneIndex);

		DistanceCount += (LTransform.GetLocation() - RTransform.GetLocation()).Size();
	}
	return DistanceCount;
}

TArray<FTransform> UDialogueAnimationUtils::SampleSequence(const UAnimSequence* Sequence, float Time, const TArray<FName>& BoneNames)
{
	TArray<FTransform> BoneTransforms;

	USkeleton& Skeletion = *Sequence->GetSkeleton();
	const FReferenceSkeleton& ReferenceSkeleton = Skeletion.GetReferenceSkeleton();

	TArray<FBoneIndexType> RequiredBones;
	TArray<FBoneIndexType> RequiredBonesWithParents;
	{
		for (const FName& BoneName : BoneNames)
		{
			int32 RequiredBoneIdx = ReferenceSkeleton.FindBoneIndex(BoneName);
			if (RequiredBoneIdx != INDEX_NONE)
			{
				RequiredBones.Add(RequiredBoneIdx);
				for (int32 ParentBoneIdx = RequiredBoneIdx; ParentBoneIdx != INDEX_NONE && !RequiredBonesWithParents.Contains(ParentBoneIdx); ParentBoneIdx = ReferenceSkeleton.GetParentIndex(ParentBoneIdx))
				{
					RequiredBonesWithParents.Add(ParentBoneIdx);
				}
			}
		}
		RequiredBonesWithParents.Sort();
	}
	check(RequiredBones.Num() == BoneNames.Num());
	FBoneContainer BoneContainer(RequiredBonesWithParents, FCurveEvaluationOption(false), Skeletion);

	FCompactPose Pose;
	FBlendedCurve Curve;
	{
		Pose.SetBoneContainer(&BoneContainer);
		FAnimExtractContext LAnimExtractContext;
		LAnimExtractContext.CurrentTime = Time;
		LAnimExtractContext.bExtractRootMotion = false;
		Sequence->GetAnimationPose(Pose, Curve, LAnimExtractContext);
	}
	FCSPose<FCompactPose> CSPose;
	CSPose.InitPose(Pose);

	float DistanceCount = 0;
	for (FBoneIndexType BoneIndex : RequiredBones)
	{
		FCompactPoseBoneIndex CompactPoseBoneIndex = BoneContainer.GetCompactPoseIndexFromSkeletonIndex(BoneIndex);
		BoneTransforms.Add(CSPose.GetComponentSpaceTransform(CompactPoseBoneIndex));
	}
	return BoneTransforms;
}
