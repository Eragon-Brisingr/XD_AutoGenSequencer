#include "Utils/AutoGenDialogueEditorStyle.h"
#include <Styling/SlateStyleRegistry.h>

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) FSlateBoxBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BORDER_BRUSH(RelativePath, ...) FSlateBorderBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define TTF_FONT(RelativePath, ...) FSlateFontInfo(RootToContentDir(RelativePath, TEXT(".ttf")), __VA_ARGS__)
#define OTF_FONT(RelativePath, ...) FSlateFontInfo(RootToContentDir(RelativePath, TEXT(".otf")), __VA_ARGS__)

FAutoGenDialogueEditorStyle::FAutoGenDialogueEditorStyle() 
	:FSlateStyleSet("AutoGenDialogueEditorStyle")
{
	const FVector2D Icon24x24(24.0f, 24.0f);
	SetContentRoot(FPaths::ProjectPluginsDir() / TEXT("AutoGenSequencer/Resources"));

	const FSlateColor DefaultForeground(FLinearColor(0.72f, 0.72f, 0.72f, 1.f));

	Set("GenDialogueSequence.OpenGenerateConfig", new IMAGE_BRUSH("Config_32x", Icon24x24));
	Set("GenDialogueSequence.OpenGenerateConfig.Small", new IMAGE_BRUSH("Config_32x", Icon24x24));

	Set("GenDialogueSequence.OpenPreviewSequence", new IMAGE_BRUSH("Preview_32x", Icon24x24));
	Set("GenDialogueSequence.OpenPreviewSequence.Small", new IMAGE_BRUSH("Preview_32x", Icon24x24));

	Set("GenDialogueSequence.OpenDialogueSequence", new IMAGE_BRUSH("Dialogue_32x", Icon24x24));
	Set("GenDialogueSequence.OpenDialogueSequence.Small", new IMAGE_BRUSH("Dialogue_32x", Icon24x24));

	Set("GenDialogueSequence.GenerateDialogueSequence", new IMAGE_BRUSH("Generate_16x", Icon24x24));
	Set("GenDialogueSequence.GenerateDialogueSequence.Small", new IMAGE_BRUSH("Generate_16x", Icon24x24));

	Set("GenDialogueSequence.RefreshStandTemplate", new IMAGE_BRUSH("StandPosition_32x", Icon24x24));
	Set("GenDialogueSequence.RefreshStandTemplate.Small", new IMAGE_BRUSH("StandPosition_32x", Icon24x24));

	Set("GenDialogueSequence.ToggleShowDebugInfo", new IMAGE_BRUSH("DebugMode_16px", Icon24x24));
	Set("GenDialogueSequence.ToggleShowDebugInfo.Small", new IMAGE_BRUSH("DebugMode_16px", Icon24x24));
	
	FSlateStyleRegistry::RegisterSlateStyle(*this);
}

FAutoGenDialogueEditorStyle::~FAutoGenDialogueEditorStyle()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*this);
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT
