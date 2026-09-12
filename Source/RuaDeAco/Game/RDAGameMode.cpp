#include "Game/RDAGameMode.h"

#include "Game/RDAGameState.h"
#include "Game/RDAPlayerController.h"
#include "Game/RDAPlayerState.h"
#include "UI/RDAHUD.h"
#include "RDACombatWorld.h"

#include "Engine/GameViewportClient.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/SpectatorPawn.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"

ARDAGameMode::ARDAGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	GameStateClass = ARDAGameState::StaticClass();
	PlayerControllerClass = ARDAPlayerController::StaticClass();
	PlayerStateClass = ARDAPlayerState::StaticClass();
	DefaultPawnClass = ASpectatorPawn::StaticClass();
	HUDClass = ARDAHUD::StaticClass();
}

ARDAGameMode::~ARDAGameMode()
{
	delete FightWorld;
	FightWorld = nullptr;
}

void ARDAGameMode::BeginPlay()
{
	Super::BeginPlay();
	bTitleScreen = true;
	TitleTicks = 0;
	StepAccumulator = 0.0f;

	if (UGameViewportClient* Viewport = GetWorld() ? GetWorld()->GetGameViewport() : nullptr)
	{
		Viewport->bDisableWorldRendering = true;
	}

	if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		PC->bAutoManageActiveCameraTarget = false;
		PC->ConsoleCommand(TEXT("r.VSync 0"));
		PC->ConsoleCommand(TEXT("t.MaxFPS 60"));
		PC->ConsoleCommand(TEXT("sg.ViewDistanceQuality 0"));
		PC->ConsoleCommand(TEXT("sg.ShadowQuality 0"));
		PC->ConsoleCommand(TEXT("sg.GlobalIlluminationQuality 0"));
		PC->ConsoleCommand(TEXT("sg.PostProcessQuality 0"));
		PC->ConsoleCommand(TEXT("sg.EffectsQuality 0"));
		PC->ConsoleCommand(TEXT("r.Streaming.PoolSize 256"));
		if (APawn* Pawn = PC->GetPawn())
		{
			Pawn->SetActorHiddenInGame(true);
			Pawn->SetActorTickEnabled(false);
			Pawn->SetActorEnableCollision(false);
		}
	}

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);
	for (AActor* Actor : Actors)
	{
		if (!Actor || Actor == this
			|| Actor->IsA(APlayerController::StaticClass())
			|| Actor->IsA(AHUD::StaticClass())
			|| Actor->IsA(AGameStateBase::StaticClass())
			|| Actor->IsA(APawn::StaticClass()))
		{
			continue;
		}
		Actor->SetActorTickEnabled(false);
		Actor->SetActorHiddenInGame(true);
	}
}

void ARDAGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	delete FightWorld;
	FightWorld = nullptr;
	Super::EndPlay(EndPlayReason);
}

void ARDAGameMode::StartMatchRuntime()
{
	delete FightWorld;
	FightWorld = new RDA::CombatWorld(RDA::GetGutoBarba(), RDA::GetRafaMare());
	bTitleScreen = false;
	StepAccumulator = 0.0f;
}

void ARDAGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}

	if (bTitleScreen)
	{
		TitleTicks += 1;
		if (PC->WasInputKeyJustPressed(EKeys::Enter) || PC->WasInputKeyJustPressed(EKeys::F)
			|| PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Bottom))
		{
			StartMatchRuntime();
		}
		return;
	}

	if (!FightWorld)
	{
		return;
	}

	RDA::InputFrame P1;
	RDA::InputFrame P2;
	ReadInputs(P1, P2);

	if (P1.pausePressed || P2.pausePressed)
	{
		FightWorld->TogglePause();
	}

	if (FightWorld->Phase() == RDA::CombatPhase::MatchOver)
	{
		if (P1.confirmPressed || P2.confirmPressed)
		{
			delete FightWorld;
			FightWorld = nullptr;
			bTitleScreen = true;
			TitleTicks = 0;
			return;
		}
		if (PC->WasInputKeyJustPressed(EKeys::R))
		{
			FightWorld->RestartMatch();
		}
		return;
	}

	StepAccumulator += DeltaSeconds;
	const float Step = RDA::kFixedStepSeconds;
	int32 Guard = 0;
	while (StepAccumulator >= Step && Guard++ < 5)
	{
		FightWorld->Step(P1, P2);
		StepAccumulator -= Step;
		ReadInputs(P1, P2);
	}
}

bool ARDAGameMode::TryGetSnapshot(RDA::WorldSnapshot& OutSnapshot) const
{
	if (!FightWorld)
	{
		return false;
	}
	OutSnapshot = FightWorld->Snapshot();
	return true;
}

void ARDAGameMode::ReadInputs(RDA::InputFrame& OutP1, RDA::InputFrame& OutP2) const
{
	OutP1 = RDA::InputFrame{};
	OutP2 = RDA::InputFrame{};
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}

	OutP1.left = PC->IsInputKeyDown(EKeys::A) || PC->IsInputKeyDown(EKeys::Gamepad_DPad_Left) || PC->GetInputAnalogKeyState(EKeys::Gamepad_LeftX) < -0.35f;
	OutP1.right = PC->IsInputKeyDown(EKeys::D) || PC->IsInputKeyDown(EKeys::Gamepad_DPad_Right) || PC->GetInputAnalogKeyState(EKeys::Gamepad_LeftX) > 0.35f;
	OutP1.up = PC->IsInputKeyDown(EKeys::W) || PC->IsInputKeyDown(EKeys::Gamepad_DPad_Up) || PC->GetInputAnalogKeyState(EKeys::Gamepad_LeftY) > 0.35f;
	OutP1.down = PC->IsInputKeyDown(EKeys::S) || PC->IsInputKeyDown(EKeys::Gamepad_DPad_Down) || PC->GetInputAnalogKeyState(EKeys::Gamepad_LeftY) < -0.35f;
	OutP1.punch = PC->IsInputKeyDown(EKeys::F) || PC->IsInputKeyDown(EKeys::Gamepad_FaceButton_Bottom);
	OutP1.kick = PC->IsInputKeyDown(EKeys::G) || PC->IsInputKeyDown(EKeys::Gamepad_FaceButton_Right);
	OutP1.block = PC->IsInputKeyDown(EKeys::R) || PC->IsInputKeyDown(EKeys::Gamepad_FaceButton_Top);
	OutP1.special = PC->IsInputKeyDown(EKeys::H) || PC->IsInputKeyDown(EKeys::Gamepad_FaceButton_Left);
	OutP1.upPressed = PC->WasInputKeyJustPressed(EKeys::W) || PC->WasInputKeyJustPressed(EKeys::Gamepad_DPad_Up);
	OutP1.punchPressed = PC->WasInputKeyJustPressed(EKeys::F) || PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Bottom);
	OutP1.kickPressed = PC->WasInputKeyJustPressed(EKeys::G) || PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Right);
	OutP1.blockPressed = PC->WasInputKeyJustPressed(EKeys::R) || PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Top);
	OutP1.pausePressed = PC->WasInputKeyJustPressed(EKeys::Escape) || PC->WasInputKeyJustPressed(EKeys::Gamepad_Special_Right);
	OutP1.confirmPressed = PC->WasInputKeyJustPressed(EKeys::Enter);
	OutP1.debugPressed = PC->WasInputKeyJustPressed(EKeys::F1);

	OutP2.left = PC->IsInputKeyDown(EKeys::Left);
	OutP2.right = PC->IsInputKeyDown(EKeys::Right);
	OutP2.up = PC->IsInputKeyDown(EKeys::Up);
	OutP2.down = PC->IsInputKeyDown(EKeys::Down);
	OutP2.punch = PC->IsInputKeyDown(EKeys::J);
	OutP2.kick = PC->IsInputKeyDown(EKeys::K);
	OutP2.block = PC->IsInputKeyDown(EKeys::U);
	OutP2.special = PC->IsInputKeyDown(EKeys::L);
	OutP2.upPressed = PC->WasInputKeyJustPressed(EKeys::Up);
	OutP2.punchPressed = PC->WasInputKeyJustPressed(EKeys::J);
	OutP2.kickPressed = PC->WasInputKeyJustPressed(EKeys::K);
	OutP2.blockPressed = PC->WasInputKeyJustPressed(EKeys::U);
	OutP2.pausePressed = PC->WasInputKeyJustPressed(EKeys::Escape);
	OutP2.confirmPressed = PC->WasInputKeyJustPressed(EKeys::Enter);
	OutP2.debugPressed = PC->WasInputKeyJustPressed(EKeys::F1);
}
