#include "Components/RDACombatComponent.h"

URDACombatComponent::URDACombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URDACombatComponent::StartMove(FName MoveId)
{
	ActiveMoveId = MoveId;
	MoveFrame = 0;
}

void URDACombatComponent::StopMove()
{
	ActiveMoveId = NAME_None;
	MoveFrame = 0;
}
