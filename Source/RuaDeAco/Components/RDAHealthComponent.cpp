#include "Components/RDAHealthComponent.h"

URDAHealthComponent::URDAHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URDAHealthComponent::ResetHealth()
{
	CurrentHealth = MaxHealth;
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

int32 URDAHealthComponent::ApplyDamage(int32 Amount)
{
	CurrentHealth = FMath::Max(0, CurrentHealth - FMath::Max(0, Amount));
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	if (CurrentHealth <= 0)
	{
		OnKnockout.Broadcast();
	}
	return CurrentHealth;
}

float URDAHealthComponent::GetHealthRatio() const
{
	return MaxHealth > 0 ? static_cast<float>(CurrentHealth) / static_cast<float>(MaxHealth) : 0.0f;
}
