#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RDAFighterStats.generated.h"

USTRUCT(BlueprintType)
struct FRDAFighterStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxHealth = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float WalkSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BackwardSpeed = 1.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float JumpSpeed = 14.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float JumpForwardSpeed = 2.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float JumpBackwardSpeed = 2.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Gravity = 0.84f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Weight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector4 Pushbox = FVector4(-15.0f, -52.0f, 30.0f, 52.0f);
};

UCLASS(BlueprintType)
class RUADEACO_API URDAFighterDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName FighterId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRDAFighterStats Stats;
};
