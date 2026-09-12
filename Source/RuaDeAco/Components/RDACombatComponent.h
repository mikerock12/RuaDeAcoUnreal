#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RDACombatComponent.generated.h"

UCLASS(ClassGroup = (RuaDeAco), meta = (BlueprintSpawnableComponent))
class RUADEACO_API URDACombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URDACombatComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	FName ActiveMoveId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	int32 MoveFrame = 0;

	UFUNCTION(BlueprintCallable, Category = "RuaDeAco")
	void StartMove(FName MoveId);

	UFUNCTION(BlueprintCallable, Category = "RuaDeAco")
	void StopMove();
};
