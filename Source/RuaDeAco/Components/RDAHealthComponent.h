#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RDAHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRDAHealthChanged, int32, Current, int32, Maximum);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRDAKnockout);

UCLASS(ClassGroup = (RuaDeAco), meta = (BlueprintSpawnableComponent))
class RUADEACO_API URDAHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URDAHealthComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	int32 MaxHealth = 1000;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	int32 CurrentHealth = 1000;

	UPROPERTY(BlueprintAssignable)
	FRDAHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FRDAKnockout OnKnockout;

	UFUNCTION(BlueprintCallable, Category = "RuaDeAco")
	void ResetHealth();

	UFUNCTION(BlueprintCallable, Category = "RuaDeAco")
	int32 ApplyDamage(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "RuaDeAco")
	float GetHealthRatio() const;
};
