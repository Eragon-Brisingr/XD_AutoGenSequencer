// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/AutoGenDialogueRuntimeSettings.h"
#include "Engine/EngineTypes.h"

#define LOCTEXT_NAMESPACE "AutoGenSequencer_Module"

UAutoGenDialogueRuntimeSettings::UAutoGenDialogueRuntimeSettings()
{
	// Camera
	CameraEvaluateTraceChannel = ETraceTypeQuery::TraceTypeQuery2;
}

#undef LOCTEXT_NAMESPACE
