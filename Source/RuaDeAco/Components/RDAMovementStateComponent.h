#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RDAMovementStateComponent.generated.h"

UENUM(BlueprintType)
enum class ERDAMoveState : uint8
{
	Idle,
	WalkForward,
	WalkBackward,
	Jump,
	Fall,
	Crouch,
	Attack,
	HitStun,
	Knockout
};

UCLASS(ClassGroup = (RuaDeAco), meta = (BlueprintSpawnableComponent))
class RUADEACO_API URDAMovementStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URDAMovementStateComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	ERDAMoveState State = ERDAMoveState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	int32 Facing = 1;
};
