// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Components/HealthSystem.h"
#include "Gameplay/Characters/FFCharacter.h"

// Sets default values for this component's properties
UHealthSystem::UHealthSystem()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	StartWithMaxHealth = true;
	// ...
}


// Called when the game starts
void UHealthSystem::BeginPlay()
{
	Super::BeginPlay();

	// Sequence 1
	AFFCharacter* Owner = Cast<AFFCharacter>(GetOwner());
	if (!Owner)
		return;
	
	CharacterRef = Owner;
	CurrentHealth = MaxHealth;

}

bool UHealthSystem::IsDead() const
{
	return CurrentHealth <= 0;
}

bool UHealthSystem::ApplyDamage(float Damage)
{
	if (IsDead())
		return true;

	float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);

	float ActualHealth = OldHealth - CurrentHealth;
	OnHealthChanged.Broadcast(ActualHealth, Damage);

	if (IsDead())
	{
		OnDeath.Broadcast();
		return true;
	}
	return false;
}

bool UHealthSystem::Heal(float HealAmount)
{
	if (IsDead())
		return false;
	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, -HealAmount);
	return true;
}

float UHealthSystem::GetCurrentHealth()
{
	return CurrentHealth;
}

void UHealthSystem::SetCurrentHealth(float Value)
{
	CurrentHealth = Value;
}

float UHealthSystem::GetMaxHealth()
{
	return MaxHealth;
}

void UHealthSystem::SetMaxHealth(float Value)
{
	MaxHealth = Value;
}

