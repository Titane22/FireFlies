// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Data/ItemData.h"
#include "GameplayTagContainer.h"
#include "WeaponData.generated.h"

class UNiagaraSystem;
class AMasterWeapon;
class UW_DynamicWeaponHUD;
class UWeaponAttackSystem;

UENUM(BlueprintType)
enum class EWeaponAttackType : uint8
{
	Hitscan,
	Projectile,
	Melee
};

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	SemiAuto    UMETA(DisplayName = "SemiAuto"),
	FullAuto    UMETA(DisplayName = "FullAuto"),
	Burst       UMETA(DisplayName = "Burst")
};

/**
 * Weapon-specific data that extends the base ItemData
 */
UCLASS()
class FF_API UWeaponData : public UItemData
{
	GENERATED_BODY()

public:
	UWeaponData();

	/** AssetManager용 PrimaryAssetId 반환 */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	
	// Weapon Details
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Details")
	TSubclassOf<UWeaponAttackSystem> AttackComponentClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire Mode Data")
	EFireMode FireMode;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire Mode Data")
	int32 BurstAmount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire Mode Data")
	float FireRate;

	// Ballistics
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ballistics")
	float BulletSpread;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ballistics")
	EWeaponAttackType AttackType;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ballistics")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ballistics")
	float MaxRange;

	// UI
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	UTexture2D* WeaponUITexture;

	// Animation	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* BodyFireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* WeaponFireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* BodyReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* WeaponReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* WeaponEquipMontage;
	// Audio
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* HitMarkerSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* KillSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* EmptySound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundAttenuation* SoundAttenuation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundConcurrency* SoundConcurrency;

	// VFX
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* MuzzleFlashVFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TSubclassOf<AActor> BulletTraceClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TSubclassOf<UUserWidget> HitMarkerUI;

	// Ammo Data
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
	FGameplayTag RequiredAmmoType;

};
