#include "Game/RDAPlayerController.h"
#include "Engine/GameViewportClient.h"

ARDAPlayerController::ARDAPlayerController()
{
	bShowMouseCursor = false;
	bAutoManageActiveCameraTarget = false;
	PrimaryActorTick.bCanEverTick = false;
}

void ARDAPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (GetWorld() && GetWorld()->GetGameViewport())
	{
		GetWorld()->GetGameViewport()->bDisableWorldRendering = true;
	}
}
