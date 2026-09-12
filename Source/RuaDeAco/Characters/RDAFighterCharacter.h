#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "RDAFighterCharacter.generated.h"

class URDAHealthComponent;
class URDACombatComponent;
class URDAHitboxComponent;
class URDAHurtboxComponent;
class URDAMovementStateComponent;
class UInputAction;
class UInputMappingContext;

UCLASS()
class RUADEACO_API ARDAFighterCharacter : public APaperCharacter
{
	GENERATED_BODY()

public:
	ARDAFighterCharacter();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	TObjectPtr<URDAHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	TObjectPtr<URDACombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	TObjectPtr<URDAHitboxComponent> HitboxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	TObjectPtr<URDAHurtboxComponent> HurtboxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	TObjectPtr<URDAMovementStateComponent> MovementStateComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco|Input")
	TObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco|Input")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco|Input")
	TObjectPtr<UInputAction> PunchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco|Input")
	TObjectPtr<UInputAction> KickAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco|Input")
	TObjectPtr<UInputAction> BlockAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco|Input")
	TObjectPtr<UInputAction> GrabAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco|Input")
	TObjectPtr<UInputAction> SpecialAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco|Input")
	TObjectPtr<UInputAction> PauseAction;

protected:
	void HandleMove(const struct FInputActionValue& Value);
	void HandleJump();
	void HandleCrouch();
	void HandlePunch();
	void HandleKick();
	void HandleBlock();
};
