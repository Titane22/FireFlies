// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponData.generated.h"

class AWeapon_Base;

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Pistol          UMETA(DisplayName = "Pistol"),
	RifleAndShotgun UMETA(DisplayName = "Rifle/Shotgun")
};

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	SemiAuto    UMETA(DisplayName = "SemiAuto"),
	FullAuto    UMETA(DisplayName = "FullAuto"),
	Burst       UMETA(DisplayName = "Burst")
};
/**
 * 
 */
UCLASS()
class FF_API UWeaponData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UWeaponData();

	// Weapon Details
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Details")
	FString WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Details")
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Details")
	TSubclassOf<AWeapon_Base> WeaponClass;

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
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ballistics")
	float MaxRange;

	// UI
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	UTexture2D* WeaponUITexture;

	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	// TSubclassOf<UW_DynamicWeaponHUD> WeaponUI;

	// Animation
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	FVector LeftHandIKOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* BodyFireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* WeaponFireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* BodyReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* WeaponReloadMontage;

	// Audio
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	USoundBase* FireSound;
};
