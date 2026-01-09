// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "WeaponSystem.generated.h"

#define COLLISION_BULLET ECollisionChannel::ECC_GameTraceChannel1

class AFFCharacter;
class USkeletalMesh;
class AMasterWeapon;
class UWeaponData;
class UUserWidget;
class UNiagaraSystem;
class USceneComponent;

USTRUCT(BlueprintType)
struct FWeapon_Data
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 CurrentAmmo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 MaxAmmo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 ClipAmmo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 DifferentAmmo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
	int32 Ammo_Count = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool ShortGun_Trace = false;
};

USTRUCT(BlueprintType)
struct FWeapon_Details
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	struct FWeapon_Data Weapon_Data = {
		/* CurrentAmmo */ 0,
		/* MaxAmmo */ 0,
		/* ClipAmmo */ 0,
		/* DifferentAmmo */ 0,
		/* Ammo_Count */ 0,
		/* ShortGun_Trace */ false
	};
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FF_API UWeaponSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponSystem();

	bool FireCheck(int32 AmmoCount);

	void FireFX(USoundBase* Sound, FVector Location, USoundAttenuation* AttenuationSettings, USoundConcurrency* Concurrency);

	void EmptyFX(USoundBase* Sound);

	void MuzzleVFX(UNiagaraSystem* NiagaraSystem, USceneComponent* AttachToComponent);

	void FireMontage(UAnimMontage* FireAnim);

	bool CheckAmmo();

	float ReloadMontage(UAnimMontage* ReloadAnim);

	void ReloadCheck();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	USkeletalMesh* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bIsDryAmmo;

	UPROPERTY()
	FWeapon_Details Weapon_Details = {
		FWeapon_Data{
			/* CurrentAmmo */ 32,
			/* MaxAmmo */ 90,
			/* ClipAmmo */ 32,
			/* DifferentAmmo */ 90,
			/* Ammo_Count */ 1,
			/* ShortGun_Trace */ false
		}
	};

	UPROPERTY()
	AFFCharacter* CharacterRef;
};
