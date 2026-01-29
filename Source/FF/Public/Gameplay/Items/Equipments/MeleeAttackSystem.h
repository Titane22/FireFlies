// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Data/WeaponData.h"
#include "Gameplay/Items/Equipments/WeaponAttackSystem.h"
#include "MeleeAttackSystem.generated.h"

class AFFCharacter;

UENUM(BlueprintType)
enum class EMeleeAttackType : uint8
{
	CanStartNextAttack		UMETA(DisplayName = "Can Start Next Attack"),
	EndCombo				UMETA(DisplayName = "End Combo"),
	CanInterrupt			UMETA(DisplayName = "Can Interrupt ")
};

/**
 * 
 */
UCLASS()
class FF_API UMeleeAttackSystem : public UWeaponAttackSystem
{
	GENERATED_BODY()

public:
	UMeleeAttackSystem();

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void PerformAttack() override;

	void StartWeaponSweep();
	
	void StopWeaponSweep();

	float GetWeaponSweepRadius();
	
	bool GetWeaponTracePoints(FVector& OutStart, FVector& OutEnd) const;

	void OnWeaponHit(FHitResult& HitResult);

	void SetComboState(EMeleeAttackType State, bool bEnable);
	void ResetCombo();

	bool bCanCombo = false;
	bool bCanBeInterrupted = false;
	bool bAttackQueued = false;

protected:
	AActor* TargetActor;

	TArray<AActor*> TargetActors;
	
protected:
	float Distance = 0.f;
	float CheckTargetsRadius = 400.f;
	float TargetYaw = 0.f;
	int32 CurrentAttackIndex = -1;
	bool bIsInterrupted = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypeQuery;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config")
	FName WeaponSweepStartSocket;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config")
	FName WeaponSweepEndSocket;

	float WeaponRadius;
	
	TArray<FComboAttack> ComboAttacks;

	FComboAttack CurrentAttack;

	FTimerHandle TimerFallingCheck;

	bool bWeaponSweepActive = false;
private:
	bool CanAttack();

	void GetNextAttack();

	void CheckTarget();

	void CalculateTargetYaw();

	void InitializeComboAttacks();

	void CauseDamage();

	void PlayCurrentAttack();

	void ExecuteQueuedAttack();
};
