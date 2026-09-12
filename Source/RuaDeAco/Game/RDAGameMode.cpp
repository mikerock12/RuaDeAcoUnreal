#include "Game/RDAGameMode.h"
#include "Game/RDAGameState.h"
#include "Game/RDAPlayerController.h"
#include "Game/RDAPlayerState.h"
#include "Characters/RDAFighterCharacter.h"

ARDAGameMode::ARDAGameMode()
{
	GameStateClass = ARDAGameState::StaticClass();
	PlayerControllerClass = ARDAPlayerController::StaticClass();
	PlayerStateClass = ARDAPlayerState::StaticClass();
	DefaultPawnClass = ARDAFighterCharacter::StaticClass();
}
