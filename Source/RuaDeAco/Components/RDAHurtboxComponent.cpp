#include "Components/RDAHurtboxComponent.h"

URDAHurtboxComponent::URDAHurtboxComponent()
{
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetHiddenInGame(true);
}
