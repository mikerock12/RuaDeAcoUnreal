#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "RDAGameState.generated.h"

UCLASS()
class RUADEACO_API ARDAGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	int32 CurrentRound = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	int32 TimeRemaining = 99;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	int32 RoundWinsP1 = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	int32 RoundWinsP2 = 0;
};
