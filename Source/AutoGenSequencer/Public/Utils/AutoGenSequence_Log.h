// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

AUTOGENSEQUENCER_API DECLARE_LOG_CATEGORY_EXTERN(AutoGenSequence_Log, Log, All);
#define AutoGenSequence_Display_Log(Format, ...) UE_LOG(AutoGenSequence_Log, Log, TEXT(Format), ##__VA_ARGS__)
#define AutoGenSequence_Warning_LOG(Format, ...) UE_LOG(AutoGenSequence_Log, Warning, TEXT(Format), ##__VA_ARGS__)
#define AutoGenSequence_Error_Log(Format, ...) UE_LOG(AutoGenSequence_Log, Error, TEXT(Format), ##__VA_ARGS__)