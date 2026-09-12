#include "Components/RDAHitboxComponent.h"

URDAHitboxComponent::URDAHitboxComponent()
{
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionObjectType(ECC_WorldDynamic);
	SetHiddenInGame(true);
	bActive = false;
}
