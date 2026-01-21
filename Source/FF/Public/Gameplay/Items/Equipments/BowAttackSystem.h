// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Items/Equipments/WeaponAttackSystem.h"
#include "BowAttackSystem.generated.h"

class AMasterProjectile;
class AMasterWeapon;
class UItemData;

/**
 * Bow Attack System
 * - 인벤토리에서 화살 검색 (태그 기반)
 * - 캐릭터의 arrow_socket에 화살 스폰
 * - 발사 시 화살 소모 및 투사체 발사
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

	/** 인벤토리에서 화살을 찾아 캐릭터의 arrow_socket에 스폰 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	void SpawnArrow();

	/** 현재 스폰된 화살 제거 */
	UFUNCTION(BlueprintCallable, Category = "Bow")
	void DestroySpawnedArrow();

	/** 화살이 장착되어 있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Bow")
	bool HasArrowEquipped() const { return SpawnedArrow != nullptr; }

	//==============================================================================
	// Charge Shot
	//==============================================================================

	/** 차지 시작 (입력 Press 시 호출) */
	UFUNCTION(BlueprintCallable, Category = "Bow|Charge")
	void StartCharge();

	/** 차지 해제 및 발사 (입력 Release 시 호출) */
	UFUNCTION(BlueprintCallable, Category = "Bow|Charge")
	void ReleaseCharge();

protected:
	/** 화살 소모 및 투사체 발사 */
	void FireArrow(float ChargeRatio);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	AMasterWeapon* OwnerWeapon = nullptr;

	/** 캐릭터 메쉬의 화살 소켓 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow|Arrow")
	FName ArrowSocketName = FName("arrow_socket");

	/** 현재 스폰된 화살 액터 */
	UPROPERTY(BlueprintReadOnly, Category = "Bow|Arrow")
	AMasterProjectile* SpawnedArrow = nullptr;

	/** 현재 장착된 화살의 ItemData (소모 시 참조용) */
	UPROPERTY(BlueprintReadOnly, Category = "Bow|Arrow")
	UItemData* CurrentArrowData = nullptr;

	//==============================================================================
	// Charge State
	//==============================================================================

	/** 최대 차지 시간 (이 이상은 더 강해지지 않음) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bow|Charge")
	float MaxChargeTime = 1.5f;

	/** 현재 차지 중인지 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Bow|Charge")
	bool bIsCharging = false;

	/** 차지 시작 시각 (월드 시간) */
	UPROPERTY(BlueprintReadOnly, Category = "Bow|Charge")
	float ChargeStartTime = 0.0f;
};
