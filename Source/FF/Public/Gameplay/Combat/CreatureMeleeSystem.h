// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Items/Equipments/MeleeAttackSystem.h"
#include "CreatureMeleeSystem.generated.h"

USTRUCT(BlueprintType)
struct FCreatureComboAttack : public FComboAttack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Creature")
	FName SweepStartSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Creature")
	FName SweepEndSocket;
};

/**
 * Melee attack system for creatures (e.g. Infector) that attack without a weapon.
 * Uses skeletal mesh sockets on the owning character instead of a weapon mesh.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FF_API UCreatureMeleeSystem : public UMeleeAttackSystem
{
	GENERATED_BODY()

public:
	UCreatureMeleeSystem();

	virtual void BeginPlay() override;

	virtual void StartWeaponSweep() override;
	virtual bool GetWeaponTracePoints(FVector& OutStart, FVector& OutEnd) const override;
	virtual bool OnWeaponHit(AActor* HitActor) override;

protected:
	virtual void InitializeComboAttacks() override;
	virtual void CauseDamage() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Creature")
	TArray<FCreatureComboAttack> CreatureComboAttacks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Creature")
	float BaseDamage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Creature")
	float CreatureHitRadius = 30.f;
};
