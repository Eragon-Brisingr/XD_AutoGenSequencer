// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

AUTOGENSEQUENCER_API DECLARE_LOG_CATEGORY_EXTERN(AutoGenSequencer_Log, Log, All);
#define AutoGenSequencer_Display_Log(Format, ...) UE_LOG(AutoGenSequencer_Log, Log, TEXT(Format), ##__VA_ARGS__)
#define AutoGenSequencer_Warning_Log(Format, ...) UE_LOG(AutoGenSequencer_Log, Warning, TEXT(Format), ##__VA_ARGS__)
#define AutoGenSequencer_Error_Log(Format, ...) UE_LOG(AutoGenSequencer_Log, Error, TEXT(Format), ##__VA_ARGS__)