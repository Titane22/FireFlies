// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Items/Equipments/WeaponAttackSystem.h"
#include "BowAttackSystem.generated.h"

/**
 * 
 */
UCLASS()
class FF_API UBowAttackSystem : public UWeaponAttackSystem
{
	GENERATED_BODY()

public:
	UBowAttackSystem();

	virtual void BeginPlay() override;
	
	virtual void PerformAttack() override;

	virtual bool CanReload() const override;
	virtual void ExecuteReload() override;
};
