#include "Characters/RDAFighterCharacter.h"
#include "Components/RDACombatComponent.h"
#include "Components/RDAHealthComponent.h"
#include "Components/RDAHitboxComponent.h"
#include "Components/RDAHurtboxComponent.h"
#include "Components/RDAMovementStateComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

ARDAFighterCharacter::ARDAFighterCharacter()
{
	HealthComponent = CreateDefaultSubobject<URDAHealthComponent>(TEXT("HealthComponent"));
	CombatComponent = CreateDefaultSubobject<URDACombatComponent>(TEXT("CombatComponent"));
	HitboxComponent = CreateDefaultSubobject<URDAHitboxComponent>(TEXT("HitboxComponent"));
	HurtboxComponent = CreateDefaultSubobject<URDAHurtboxComponent>(TEXT("HurtboxComponent"));
	MovementStateComponent = CreateDefaultSubobject<URDAMovementStateComponent>(TEXT("MovementStateComponent"));
}

void ARDAFighterCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (const APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (MappingContext)
			{
				Subsystem->AddMappingContext(MappingContext, 0);
			}
		}
	}
}

void ARDAFighterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* Enhanced = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction) Enhanced->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARDAFighterCharacter::HandleMove);
		if (JumpAction) Enhanced->BindAction(JumpAction, ETriggerEvent::Started, this, &ARDAFighterCharacter::HandleJump);
		if (CrouchAction) Enhanced->BindAction(CrouchAction, ETriggerEvent::Started, this, &ARDAFighterCharacter::HandleCrouch);
		if (PunchAction) Enhanced->BindAction(PunchAction, ETriggerEvent::Started, this, &ARDAFighterCharacter::HandlePunch);
		if (KickAction) Enhanced->BindAction(KickAction, ETriggerEvent::Started, this, &ARDAFighterCharacter::HandleKick);
		if (BlockAction) Enhanced->BindAction(BlockAction, ETriggerEvent::Started, this, &ARDAFighterCharacter::HandleBlock);
	}
}

void ARDAFighterCharacter::HandleMove(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), Axis.X);
}

void ARDAFighterCharacter::HandleJump()
{
	Jump();
}

void ARDAFighterCharacter::HandleCrouch()
{
}

void ARDAFighterCharacter::HandlePunch()
{
	if (CombatComponent)
	{
		CombatComponent->StartMove(TEXT("punch"));
	}
}

void ARDAFighterCharacter::HandleKick()
{
	if (CombatComponent)
	{
		CombatComponent->StartMove(TEXT("kick"));
	}
}

void ARDAFighterCharacter::HandleBlock()
{
}
