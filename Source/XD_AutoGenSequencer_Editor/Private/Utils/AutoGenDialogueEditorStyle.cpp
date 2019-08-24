#include "AutoGenDialogueEditorStyle.h"
#include "Styling/SlateStyleRegistry.h"

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) FSlateBoxBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BORDER_BRUSH(RelativePath, ...) FSlateBorderBrush(RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define TTF_FONT(RelativePath, ...) FSlateFontInfo(RootToContentDir(RelativePath, TEXT(".ttf")), __VA_ARGS__)
#define OTF_FONT(RelativePath, ...) FSlateFontInfo(RootToContentDir(RelativePath, TEXT(".otf")), __VA_ARGS__)

FAutoGenDialogueEditorStyle::FAutoGenDialogueEditorStyle() 
	:FSlateStyleSet("AutoGenDialogueEditorStyle")
{
	const FVector2D Icon10x10(10.0f, 10.0f);
	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon24x24(24.0f, 24.0f);
	const FVector2D Icon32x32(32.0f, 32.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);
	SetContentRoot(FPaths::ProjectPluginsDir() / TEXT("XD_AutoGenSequencer/Resources"));

	const FSlateColor DefaultForeground(FLinearColor(0.72f, 0.72f, 0.72f, 1.f));

	Set("AutoGenDialogue.Config", new IMAGE_BRUSH("Config_32x", Icon24x24));
	Set("AutoGenDialogue.Preview", new IMAGE_BRUSH("Preview_32x", Icon24x24));
	Set("AutoGenDialogue.Dialogue", new IMAGE_BRUSH("Dialogue_32x", Icon24x24));
	Set("AutoGenDialogue.Generate", new IMAGE_BRUSH("Generate_16x", Icon24x24));
	Set("AutoGenDialogue.StandPosition", new IMAGE_BRUSH("StandPosition_32x", Icon24x24));

	FSlateStyleRegistry::RegisterSlateStyle(*this);
}

FAutoGenDialogueEditorStyle::~FAutoGenDialogueEditorStyle()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*this);
}

FSlateIcon FAutoGenDialogueEditorStyle::GetConfigIcon() const
{
	return FSlateIcon(FAutoGenDialogueEditorStyle::Get().GetStyleSetName(), "AutoGenDialogue.Config", "AutoGenDialogue.Config");
}

FSlateIcon FAutoGenDialogueEditorStyle::GetPreviewIcon() const
{
	return FSlateIcon(FAutoGenDialogueEditorStyle::Get().GetStyleSetName(), "AutoGenDialogue.Preview", "AutoGenDialogue.Preview");
}

FSlateIcon FAutoGenDialogueEditorStyle::GetDialogueIcon() const
{
	return FSlateIcon(FAutoGenDialogueEditorStyle::Get().GetStyleSetName(), "AutoGenDialogue.Dialogue", "AutoGenDialogue.Dialogue");
}

FSlateIcon FAutoGenDialogueEditorStyle::GetGenerateIcon() const
{
	return FSlateIcon(FAutoGenDialogueEditorStyle::Get().GetStyleSetName(), "AutoGenDialogue.Generate", "AutoGenDialogue.Generate");
}

FSlateIcon FAutoGenDialogueEditorStyle::GetStandpositionIcon() const
{
	return FSlateIcon(FAutoGenDialogueEditorStyle::Get().GetStyleSetName(), "AutoGenDialogue.StandPosition", "AutoGenDialogue.StandPosition");
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT
