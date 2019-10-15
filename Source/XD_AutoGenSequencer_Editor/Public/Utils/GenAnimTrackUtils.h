// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FrameRate.h"
#include "NoExportTypes.h"
#include "MovieSceneFloatChannel.h"


namespace GenAnimTrackUtils
{
	FFrameNumber SecondToFrameNumber(float Second, const FFrameRate FrameRate)
	{
		return (Second * FrameRate).GetFrame();
	}

	void SetBlendInValue(FMovieSceneFloatChannel& AnimWeight, const FFrameRate FrameRate, FFrameNumber StartFrameNumber, float BlendTime, float StartWeight, float EndWeight)
	{
		const FFrameNumber BlendFrameNumber = SecondToFrameNumber(BlendTime, FrameRate);
		AnimWeight.AddCubicKey(StartFrameNumber, StartWeight);
		AnimWeight.AddCubicKey(StartFrameNumber + BlendFrameNumber, EndWeight);
	}

	void SetBlendOutValue(FMovieSceneFloatChannel& AnimWeight, const FFrameRate FrameRate, FFrameNumber EndFrameNumber, float BlendTime, float StartWeight, float EndWeight)
	{
		const FFrameNumber BlendFrameNumber = SecondToFrameNumber(BlendTime, FrameRate);
		AnimWeight.AddCubicKey(EndFrameNumber - BlendFrameNumber, StartWeight);
		AnimWeight.AddCubicKey(EndFrameNumber, EndWeight);
	}

	void SetBlendInOutValue(FMovieSceneFloatChannel& AnimWeight, const FFrameRate FrameRate, FFrameNumber StartFrameNumber, float StartBlendTime, FFrameNumber EndFrameNumber, float EndBlendTime)
	{
		SetBlendInValue(AnimWeight, FrameRate, StartFrameNumber, StartBlendTime, 0.f, 1.f);
		SetBlendOutValue(AnimWeight, FrameRate, EndFrameNumber, EndBlendTime, 1.f, 0.f);
	}
};

