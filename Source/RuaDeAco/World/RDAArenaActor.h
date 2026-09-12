#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RDAArenaActor.generated.h"

UCLASS()
class RUADEACO_API ARDAArenaActor : public AActor
{
	GENERATED_BODY()

public:
	ARDAArenaActor();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	FName ArenaId = TEXT("cais-da-cidade");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	float GroundY = 304.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	float StageLeft = 36.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	float StageRight = 604.0f;
};
