#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RDAAttackData.generated.h"

UENUM(BlueprintType)
enum class ERDAHitLevel : uint8
{
	High,
	Mid,
	Low,
	Overhead
};

USTRUCT(BlueprintType)
struct FRDAHitboxData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ActiveFrom = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ActiveTo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector4 Box = FVector4(0.0f, 0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ERDAHitLevel Level = ERDAHitLevel::Mid;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Damage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 HitStun = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 BlockStun = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 HitStop = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D Knockback = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bKnockdown = false;
};

USTRUCT(BlueprintType)
struct FRDAAttackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName AttackId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 StartupFrames = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ActiveFrames = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RecoveryFrames = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TotalFrames = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAir = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FRDAHitboxData Hitbox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCanCancel = false;
};

UCLASS(BlueprintType)
class RUADEACO_API URDAAttackDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FRDAAttackData> Attacks;
};
