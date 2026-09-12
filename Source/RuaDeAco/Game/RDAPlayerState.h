#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "RDAPlayerState.generated.h"

UCLASS()
class RUADEACO_API ARDAPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	int32 RoundWins = 0;
};
