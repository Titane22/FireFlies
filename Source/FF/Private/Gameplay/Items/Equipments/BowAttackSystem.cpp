// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/Equipments/BowAttackSystem.h"

UBowAttackSystem::UBowAttackSystem()
{
}

void UBowAttackSystem::BeginPlay()
{
	Super::BeginPlay();
}

void UBowAttackSystem::PerformAttack()
{
}

bool UBowAttackSystem::CanReload() const
{
	return false;
}

void UBowAttackSystem::ExecuteReload()
{
}
