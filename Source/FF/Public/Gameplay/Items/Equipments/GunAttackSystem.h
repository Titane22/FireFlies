// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponAttackSystem.h"
#include "GunAttackSystem.generated.h"

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
class FF_API UGunAttackSystem : public UWeaponAttackSystem
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGunAttackSystem();
	
	virtual void BeginPlay() override;
	
	virtual void PerformAttack() override;

	virtual bool CanReload() const override;
	virtual void ExecuteReload() override;
	virtual int32 GetCurrentAmmo() const override { return Weapon_Details.Weapon_Data.CurrentAmmo; }
	virtual int32 GetMaxAmmo() const override { return Weapon_Details.Weapon_Data.MaxAmmo; }
	
	void FireBullet(FHitResult Hit, bool bReturnHit);

	bool FireCheck(int32 AmmoCount);

	void FireFX(USoundBase* Sound, FVector Location, USoundAttenuation* AttenuationSettings, USoundConcurrency* Concurrency);

	void EmptyFX(USoundBase* Sound);

	void MuzzleVFX(UNiagaraSystem* NiagaraSystem, USceneComponent* AttachToComponent);

	void FireMontage(UAnimMontage* FireAnim);

	bool CheckAmmo() const;

	float ReloadMontage(UAnimMontage* ReloadAnim);

	void ReloadCheck();
	
	void RandPointInCircle(float Radius, float& PointX, float& PointY);
	
	void FireBlankTracer();

	void PlayFireEffect();
private:
	// Fire helper functions
	void ApplyCameraShake(APlayerController* PC);
	bool PerformCameraTrace(APlayerCameraManager* CameraManager, FHitResult& OutHitResult);
	void ExecuteFireSequence(const FHitResult& CameraHitResult);
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	AMasterWeapon* OwnerWeapon;	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bIsDryAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bAutoReload;
	
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
};
