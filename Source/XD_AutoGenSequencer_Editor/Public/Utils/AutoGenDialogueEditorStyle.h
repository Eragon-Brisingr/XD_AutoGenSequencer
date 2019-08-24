// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SlateStyle.h"
#include "SlateIcon.h"

class FAutoGenDialogueEditorStyle
	: public FSlateStyleSet
{
public:
	FAutoGenDialogueEditorStyle();

	static FAutoGenDialogueEditorStyle& Get()
	{
		static FAutoGenDialogueEditorStyle Inst;
		return Inst;
	}

	~FAutoGenDialogueEditorStyle();
public:
	FSlateIcon GetConfigIcon() const;
	FSlateIcon GetPreviewIcon() const;
	FSlateIcon GetDialogueIcon() const;
	FSlateIcon GetGenerateIcon() const;
	FSlateIcon GetStandpositionIcon() const;
};
