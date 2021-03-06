﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/DialogueAnimationUtils.h"
#include <Animation/AnimSequence.h>
#include <BonePose.h>
#include <Animation/AnimCurveTypes.h>
#include <Animation/AnimationAsset.h>
#include <Animation/Skeleton.h>
#include <AnimationRuntime.h>

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
		FAnimExtractContext AnimExtractContext;
		AnimExtractContext.CurrentTime = Time;
		AnimExtractContext.bExtractRootMotion = false;
		Sequence->GetAnimationPose(Pose, Curve, AnimExtractContext);
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
