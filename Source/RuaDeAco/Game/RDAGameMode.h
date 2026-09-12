#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RDAGameMode.generated.h"

UCLASS()
class RUADEACO_API ARDAGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARDAGameMode();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	int32 RoundsToWin = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	int32 RoundTimeSeconds = 99;
};
