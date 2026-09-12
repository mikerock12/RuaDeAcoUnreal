#include "UI/RDAHUD.h"

#include "Game/RDAGameMode.h"
#include "RDACombatWorld.h"

#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/Texture2D.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "TextureResource.h"

namespace
{
	constexpr int32 kAnimCount = 23;

	struct FLocalAnimSpec
	{
		RDA::AnimId Id;
		const TCHAR* File;
		int32 Fps;
		bool bLoop;
	};

	const FLocalAnimSpec GutoSpecs[] = {
		{RDA::AnimId::Idle, TEXT("idle.png"), 5, true},
		{RDA::AnimId::Walk, TEXT("corrida.png"), 7, true},
		{RDA::AnimId::WalkBackward, TEXT("walk-backward.png"), 6, true},
		{RDA::AnimId::JumpNeutral, TEXT("jump-neutral.png"), 6, false},
		{RDA::AnimId::JumpForward, TEXT("jump-forward.png"), 6, false},
		{RDA::AnimId::JumpBackward, TEXT("jump-backward.png"), 6, false},
		{RDA::AnimId::Fall, TEXT("fall.png"), 6, false},
		{RDA::AnimId::Landing, TEXT("landing.png"), 8, false},
		{RDA::AnimId::Crouch, TEXT("crouch.png"), 5, false},
		{RDA::AnimId::StandingLight, TEXT("standing-light.png"), 9, false},
		{RDA::AnimId::StandingHeavy, TEXT("standing-heavy.png"), 7, false},
		{RDA::AnimId::ForwardHeavy, TEXT("forward-heavy.png"), 7, false},
		{RDA::AnimId::CrouchLight, TEXT("crouch-light.png"), 9, false},
		{RDA::AnimId::CrouchHeavy, TEXT("crouch-heavy.png"), 7, false},
		{RDA::AnimId::AirLight, TEXT("air-light-neutral.png"), 9, false},
		{RDA::AnimId::AirHeavy, TEXT("air-heavy-neutral.png"), 7, false},
		{RDA::AnimId::BlockStanding, TEXT("block-standing.png"), 7, false},
		{RDA::AnimId::BlockCrouching, TEXT("block-crouching.png"), 7, false},
		{RDA::AnimId::Hit, TEXT("hit.png"), 10, false},
		{RDA::AnimId::Knockdown, TEXT("knockdown.png"), 6, false},
		{RDA::AnimId::WakeUp, TEXT("wake-up.png"), 7, false},
		{RDA::AnimId::Knockout, TEXT("knockout.png"), 5, false},
		{RDA::AnimId::Victory, TEXT("victory.png"), 6, true},
	};

	const FLocalAnimSpec RafaSpecs[] = {
		{RDA::AnimId::Idle, TEXT("idle.png"), 6, true},
		{RDA::AnimId::Walk, TEXT("corrida.png"), 10, true},
		{RDA::AnimId::WalkBackward, TEXT("walk-backward.png"), 8, true},
		{RDA::AnimId::JumpNeutral, TEXT("jump-neutral.png"), 8, false},
		{RDA::AnimId::JumpForward, TEXT("jump-forward.png"), 8, false},
		{RDA::AnimId::JumpBackward, TEXT("jump-backward.png"), 8, false},
		{RDA::AnimId::Fall, TEXT("fall.png"), 8, false},
		{RDA::AnimId::Landing, TEXT("landing.png"), 12, false},
		{RDA::AnimId::Crouch, TEXT("crouch.png"), 6, false},
		{RDA::AnimId::StandingLight, TEXT("standing-light.png"), 12, false},
		{RDA::AnimId::StandingHeavy, TEXT("standing-heavy.png"), 9, false},
		{RDA::AnimId::ForwardHeavy, TEXT("forward-heavy.png"), 9, false},
		{RDA::AnimId::CrouchLight, TEXT("crouch-light.png"), 12, false},
		{RDA::AnimId::CrouchHeavy, TEXT("crouch-heavy.png"), 9, false},
		{RDA::AnimId::AirLight, TEXT("air-light-neutral.png"), 12, false},
		{RDA::AnimId::AirHeavy, TEXT("air-heavy-neutral.png"), 9, false},
		{RDA::AnimId::BlockStanding, TEXT("block-standing.png"), 8, false},
		{RDA::AnimId::BlockCrouching, TEXT("block-crouching.png"), 8, false},
		{RDA::AnimId::Hit, TEXT("hit.png"), 10, false},
		{RDA::AnimId::Knockdown, TEXT("knockdown.png"), 7, false},
		{RDA::AnimId::WakeUp, TEXT("wake-up.png"), 9, false},
		{RDA::AnimId::Knockout, TEXT("knockout.png"), 6, false},
		{RDA::AnimId::Victory, TEXT("victory.png"), 7, true},
	};

	const FLinearColor Ink(0.008f, 0.016f, 0.04f, 1.0f);
	const FLinearColor Cyan(0.125f, 0.78f, 0.90f, 1.0f);
	const FLinearColor Pink(0.83f, 0.28f, 0.44f, 1.0f);
	const FLinearColor Gold(0.91f, 0.72f, 0.29f, 1.0f);
	const FLinearColor Ivory(0.95f, 0.91f, 0.74f, 1.0f);
	const FLinearColor Muted(0.44f, 0.53f, 0.60f, 1.0f);
	const FLinearColor Danger(0.85f, 0.26f, 0.31f, 1.0f);
}

void ARDAHUD::BeginPlay()
{
	Super::BeginPlay();
	EnsureLoaded();
}

UTexture2D* ARDAHUD::LoadPng(const FString& Path) const
{
	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *Path) || FileData.Num() == 0)
	{
		return nullptr;
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	TSharedPtr<IImageWrapper> Wrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
	if (!Wrapper.IsValid() || !Wrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		return nullptr;
	}

	TArray<uint8> Raw;
	if (!Wrapper->GetRaw(ERGBFormat::BGRA, 8, Raw))
	{
		return nullptr;
	}

	const int32 Width = Wrapper->GetWidth();
	const int32 Height = Wrapper->GetHeight();
	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!Texture || !Texture->GetPlatformData() || Texture->GetPlatformData()->Mips.Num() == 0)
	{
		return nullptr;
	}

	Texture->SRGB = true;
	Texture->Filter = TF_Nearest;
	Texture->AddressX = TA_Clamp;
	Texture->AddressY = TA_Clamp;
	Texture->NeverStream = true;
	Texture->CompressionSettings = TC_EditorIcon;

	void* MipData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(MipData, Raw.GetData(), Raw.Num());
	Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
	Texture->UpdateResource();
	Texture->AddToRoot();
	return Texture;
}

void ARDAHUD::EnsureLoaded()
{
	if (bLoaded)
	{
		return;
	}
	bLoaded = true;

	const FString ArtRoot = FPaths::ProjectContentDir() / TEXT("RuaDeAco") / TEXT("Art");
	StageTexture = LoadPng(ArtRoot / TEXT("Environments") / TEXT("CaisDaCidade") / TEXT("background.png"));

	auto LoadFighter = [this](TArray<FAnimSheet>& OutSheets, const FString& Folder, int32 FrameSize, const FLocalAnimSpec* Specs, int32 Count)
	{
		OutSheets.SetNum(kAnimCount);
		for (int32 i = 0; i < Count; ++i)
		{
			const int32 Index = static_cast<int32>(Specs[i].Id);
			FAnimSheet& Sheet = OutSheets[Index];
			Sheet.FrameSize = FrameSize;
			Sheet.Fps = Specs[i].Fps;
			Sheet.bLoop = Specs[i].bLoop;
			Sheet.Texture = LoadPng(Folder / Specs[i].File);
			if (Sheet.Texture)
			{
				Sheet.Frames = FMath::Max(1, Sheet.Texture->GetSizeX() / FMath::Max(1, FrameSize));
			}
		}
	};

	LoadFighter(GutoSheets, ArtRoot / TEXT("Characters") / TEXT("GutoBarba"), 288, GutoSpecs, UE_ARRAY_COUNT(GutoSpecs));
	LoadFighter(RafaSheets, ArtRoot / TEXT("Characters") / TEXT("RafaMare"), 256, RafaSpecs, UE_ARRAY_COUNT(RafaSpecs));
}

float ARDAHUD::GetScale() const
{
	const float ScaleX = Canvas ? Canvas->SizeX / static_cast<float>(LogicalW) : 1.0f;
	const float ScaleY = Canvas ? Canvas->SizeY / static_cast<float>(LogicalH) : 1.0f;
	return static_cast<float>(FMath::Max(1, FMath::FloorToInt(FMath::Min(ScaleX, ScaleY))));
}

FVector2D ARDAHUD::GetOrigin() const
{
	const float Scale = GetScale();
	const float DrawW = LogicalW * Scale;
	const float DrawH = LogicalH * Scale;
	return FVector2D(
		(Canvas->SizeX - DrawW) * 0.5f,
		(Canvas->SizeY - DrawH) * 0.5f);
}

FVector2D ARDAHUD::LogicalToScreen(float X, float Y) const
{
	const FVector2D Origin = GetOrigin();
	const float Scale = GetScale();
	return FVector2D(Origin.X + X * Scale, Origin.Y + Y * Scale);
}

void ARDAHUD::DrawLogicalQuad(float X, float Y, float W, float H, FLinearColor Color)
{
	const FVector2D P = LogicalToScreen(X, Y);
	const float Scale = GetScale();
	DrawRect(Color, P.X, P.Y, W * Scale, H * Scale);
}

void ARDAHUD::DrawSheet(const FAnimSheet& Sheet, float WorldX, float WorldY, int32 Facing, int32 Frame, int32 SolePad)
{
	if (!Sheet.Texture || !Canvas)
	{
		return;
	}

	const float Scale = GetScale();
	const FVector2D Origin = GetOrigin();
	const float Size = static_cast<float>(Sheet.FrameSize);
	const float DrawX = Origin.X + WorldX * Scale;
	const float DrawY = Origin.Y + (WorldY + SolePad) * Scale;
	const float DestW = Size * Scale;
	const float DestH = Size * Scale;
	const float DestLeft = DrawX - DestW * 0.5f;
	const float DestTop = DrawY - DestH;

	const int32 UseFrame = FMath::Clamp(Frame, 0, FMath::Max(0, Sheet.Frames - 1));
	const float TexW = static_cast<float>(Sheet.Texture->GetSizeX());
	const float TexH = static_cast<float>(Sheet.Texture->GetSizeY());
	float U = (UseFrame * Sheet.FrameSize) / TexW;
	float UL = Sheet.FrameSize / TexW;
	const float V = 0.0f;
	const float VL = Sheet.FrameSize / TexH;

	float ScreenX = DestLeft;
	float ScreenW = DestW;
	if (Facing < 0)
	{
		ScreenX = DestLeft + DestW;
		ScreenW = -DestW;
	}

	DrawTexture(Sheet.Texture, ScreenX, DestTop, ScreenW, DestH, U, V, UL, VL, FLinearColor::White, BLEND_Translucent);
}

void ARDAHUD::DrawFighter(int32 FighterIndex, const RDA::FighterSnapshot& Snap)
{
	const TArray<FAnimSheet>& Sheets = FighterIndex == 0 ? GutoSheets : RafaSheets;
	const int32 AnimIndex = static_cast<int32>(Snap.anim);
	if (!Sheets.IsValidIndex(AnimIndex))
	{
		return;
	}
	const FAnimSheet& Sheet = Sheets[AnimIndex];
	int32 Frame = 0;
	if (Sheet.Frames > 1)
	{
		if (Snap.attacking)
		{
			RDA::MoveDef Move;
			Move.totalFrames = FMath::Max(1, Snap.moveTotalFrames);
			Move.hitbox.activeFrom = Snap.hitActiveFrom;
			Move.hitbox.activeTo = Snap.hitActiveTo;
			Frame = RDA::MoveAnimFrame(Move, Snap.moveFrame, Sheet.Frames);
		}
		else
		{
			const int32 Ticks = FMath::Max(1, 60 / FMath::Max(1, Sheet.Fps));
			Frame = Snap.stateFrame / Ticks;
			Frame = Sheet.bLoop ? Frame % Sheet.Frames : FMath::Min(Sheet.Frames - 1, Frame);
		}
	}

	const int32 SolePad = FighterIndex == 0 ? RDA::GetGutoBarba().visualSolePad : RDA::GetRafaMare().visualSolePad;
	DrawSheet(Sheet, Snap.x, Snap.y, Snap.facing, Frame, SolePad);

	if (Snap.hitboxesWorld.empty() && Snap.hurtboxesWorld.empty())
	{
		return;
	}
}

void ARDAHUD::DrawHudOverlay(const RDA::WorldSnapshot& Snap)
{
	DrawLogicalQuad(0, 0, static_cast<float>(LogicalW), 52, FLinearColor(0.02f, 0.03f, 0.07f, 0.85f));

	const float R1 = Snap.fighters[0].maxHealth > 0 ? Snap.fighters[0].health / static_cast<float>(Snap.fighters[0].maxHealth) : 0.0f;
	const float R2 = Snap.fighters[1].maxHealth > 0 ? Snap.fighters[1].health / static_cast<float>(Snap.fighters[1].maxHealth) : 0.0f;

	DrawLogicalQuad(30, 18, 196, 8, FLinearColor(0.08f, 0.11f, 0.16f, 1));
	DrawLogicalQuad(30, 18, 196 * R1, 8, R1 < 0.25f ? Danger : Cyan);
	DrawLogicalQuad(LogicalW - 226, 18, 196, 8, FLinearColor(0.08f, 0.11f, 0.16f, 1));
	DrawLogicalQuad(LogicalW - 226 + 196 * (1.0f - R2), 18, 196 * R2, 8, R2 < 0.25f ? Danger : Pink);

	if (Canvas)
	{
		const float Scale = GetScale();
		FCanvasTextItem Timer(LogicalToScreen(300, 6), FText::FromString(FString::Printf(TEXT("%02d"), Snap.timeSeconds)), GEngine->GetLargeFont(), Gold);
		Timer.Scale = FVector2D(Scale, Scale);
		Canvas->DrawItem(Timer);

		FCanvasTextItem N1(LogicalToScreen(30, 6), FText::FromString(UTF8_TO_TCHAR(Snap.fighters[0].name)), GEngine->GetSmallFont(), Ivory);
		N1.Scale = FVector2D(Scale * 0.9f, Scale * 0.9f);
		Canvas->DrawItem(N1);

		FCanvasTextItem N2(LogicalToScreen(418, 6), FText::FromString(UTF8_TO_TCHAR(Snap.fighters[1].name)), GEngine->GetSmallFont(), Ivory);
		N2.Scale = FVector2D(Scale * 0.9f, Scale * 0.9f);
		Canvas->DrawItem(N2);

		if (Snap.banner && Snap.banner[0] != '\0')
		{
			FCanvasTextItem Banner(LogicalToScreen(240, 120), FText::FromString(UTF8_TO_TCHAR(Snap.banner)), GEngine->GetLargeFont(), Gold);
			Banner.Scale = FVector2D(Scale * 1.4f, Scale * 1.4f);
			Canvas->DrawItem(Banner);
		}

		FCanvasTextItem Help(LogicalToScreen(8, static_cast<float>(LogicalH - 18)), FText::FromString(TEXT("P1 WASD F/G/R    P2 SETAS J/K/U    ESC pausa    F1 boxes")), GEngine->GetSmallFont(), Muted);
		Help.Scale = FVector2D(Scale * 0.8f, Scale * 0.8f);
		Canvas->DrawItem(Help);
	}

	if (Snap.debugBoxes)
	{
		auto DrawBox = [this](const RDA::Rect& Box, FLinearColor Color)
		{
			DrawLogicalQuad(Box.x, Box.y, Box.width, 1, Color);
			DrawLogicalQuad(Box.x, Box.y + Box.height, Box.width, 1, Color);
			DrawLogicalQuad(Box.x, Box.y, 1, Box.height, Color);
			DrawLogicalQuad(Box.x + Box.width, Box.y, 1, Box.height, Color);
		};
		for (int32 i = 0; i < 2; ++i)
		{
			DrawBox(Snap.fighters[i].pushboxWorld, FLinearColor(0.3f, 0.7f, 1, 0.9f));
			for (const RDA::Rect& Hurt : Snap.fighters[i].hurtboxesWorld)
			{
				DrawBox(Hurt, FLinearColor(0.3f, 1, 0.3f, 0.9f));
			}
			for (const RDA::Rect& Hit : Snap.fighters[i].hitboxesWorld)
			{
				DrawLogicalQuad(Hit.x, Hit.y, Hit.width, Hit.height, FLinearColor(1, 0.1f, 0.1f, 0.35f));
			}
		}
	}
}

void ARDAHUD::DrawTitle()
{
	EnsureLoaded();
	DrawLogicalQuad(0, 0, static_cast<float>(LogicalW), static_cast<float>(LogicalH), Ink);
	if (StageTexture)
	{
		const FVector2D Origin = GetOrigin();
		const float Scale = GetScale();
		DrawTexture(StageTexture, Origin.X, Origin.Y, LogicalW * Scale, LogicalH * Scale, 0, 0, 1, 1, FLinearColor::White, BLEND_Opaque);
		DrawLogicalQuad(0, 0, static_cast<float>(LogicalW), static_cast<float>(LogicalH), FLinearColor(0, 0, 0.05f, 0.45f));
	}

	const ARDAGameMode* GameMode = Cast<ARDAGameMode>(GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr);
	const int32 Ticks = GameMode ? GameMode->GetTitleTicks() : 0;

	RDA::FighterSnapshot Guto{};
	Guto.id = RDA::FighterId::GutoBarba;
	Guto.x = 188;
	Guto.y = RDA::kGroundY;
	Guto.facing = 1;
	Guto.stateFrame = Ticks;
	Guto.anim = RDA::AnimId::Idle;
	RDA::FighterSnapshot Rafa = Guto;
	Rafa.id = RDA::FighterId::RafaMare;
	Rafa.x = 452;
	Rafa.facing = -1;
	DrawFighter(0, Guto);
	DrawFighter(1, Rafa);

	if (Canvas)
	{
		const float Scale = GetScale();
		FCanvasTextItem Title(LogicalToScreen(160, 20), FText::FromString(TEXT("RUA DE ACO")), GEngine->GetLargeFont(), Cyan);
		Title.Scale = FVector2D(Scale * 1.6f, Scale * 1.6f);
		Canvas->DrawItem(Title);

		FCanvasTextItem Sub(LogicalToScreen(180, 48), FText::FromString(TEXT("UNREAL C++  /  PROTOTIPO 0.1")), GEngine->GetSmallFont(), Gold);
		Sub.Scale = FVector2D(Scale, Scale);
		Canvas->DrawItem(Sub);

		if ((Ticks / 40) % 2 == 0)
		{
			FCanvasTextItem Start(LogicalToScreen(220, 280), FText::FromString(TEXT("ENTER PARA LUTAR")), GEngine->GetLargeFont(), Gold);
			Start.Scale = FVector2D(Scale, Scale);
			Canvas->DrawItem(Start);
		}

		FCanvasTextItem Help(LogicalToScreen(70, 320), FText::FromString(TEXT("P1 WASD F soco G chute R defesa   P2 setas J/K/U")), GEngine->GetSmallFont(), Muted);
		Help.Scale = FVector2D(Scale * 0.85f, Scale * 0.85f);
		Canvas->DrawItem(Help);
	}
}

void ARDAHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas)
	{
		return;
	}

	EnsureLoaded();
	DrawRect(Ink, 0, 0, Canvas->SizeX, Canvas->SizeY);

	const ARDAGameMode* GameMode = Cast<ARDAGameMode>(GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr);
	if (!GameMode || GameMode->IsTitleScreen())
	{
		DrawTitle();
		return;
	}

	RDA::WorldSnapshot Snap;
	if (!GameMode->TryGetSnapshot(Snap))
	{
		return;
	}

	const FVector2D Origin = GetOrigin();
	const float Scale = GetScale();
	if (StageTexture)
	{
		DrawTexture(StageTexture, Origin.X, Origin.Y, LogicalW * Scale, LogicalH * Scale, 0, 0, 1, 1, FLinearColor::White, BLEND_Opaque);
	}

	DrawFighter(0, Snap.fighters[0]);
	DrawFighter(1, Snap.fighters[1]);
	DrawHudOverlay(Snap);
}
