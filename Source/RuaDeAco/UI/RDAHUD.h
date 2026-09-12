#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RDATypes.h"
#include "RDAHUD.generated.h"

class UTexture2D;

UCLASS()
class RUADEACO_API ARDAHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
	virtual void BeginPlay() override;

private:
	struct FAnimSheet
	{
		TObjectPtr<UTexture2D> Texture;
		int32 FrameSize = 256;
		int32 Frames = 4;
		int32 Fps = 6;
		bool bLoop = false;
	};

	void EnsureLoaded();
	UTexture2D* LoadPng(const FString& Path) const;
	void DrawLogicalQuad(float X, float Y, float W, float H, FLinearColor Color);
	void DrawSheet(const FAnimSheet& Sheet, float WorldX, float WorldY, int32 Facing, int32 Frame, int32 SolePad);
	void DrawFighter(int32 FighterIndex, const RDA::FighterSnapshot& Snap);
	void DrawHudOverlay(const RDA::WorldSnapshot& Snap);
	void DrawTitle();
	FVector2D LogicalToScreen(float X, float Y) const;
	float GetScale() const;
	FVector2D GetOrigin() const;

	bool bLoaded = false;
	TObjectPtr<UTexture2D> StageTexture;
	TArray<FAnimSheet> GutoSheets;
	TArray<FAnimSheet> RafaSheets;
	int32 LogicalW = 640;
	int32 LogicalH = 360;
};
