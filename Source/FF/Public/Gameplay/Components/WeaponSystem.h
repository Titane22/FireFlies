// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Library/AnimationState.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "WeaponSystem.generated.h"

class UWeaponData;
class AFFCharacter;
class AWeapon_Base;

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

UENUM(BlueprintType)
enum class EWeaponSlot : uint8
{
	Primary     UMETA(DisplayName = "Primary"),
	Handgun     UMETA(DisplayName = "Handgun")
};

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Equip       UMETA(DisplayName = "Equip"),
	Unequip     UMETA(DisplayName = "Unequip")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FF_API UWeaponSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponSystem();

	UFUNCTION()
	void SetWeaponState(TSubclassOf<AWeapon_Base> ToSetWeaponClass, EAnimationState ToSetAnimation, EWeaponState CurWeaponState, FName ToSetEquipSocketName, FName ToSetUnequipSocketName, EWeaponSlot WeaponSlot = EWeaponSlot::Primary);

	UFUNCTION()
	void EquipWeapon(FName SocketName, EWeaponSlot WeaponSlot);

	UFUNCTION()
	void UnequipWeapon(FName SocketName, EWeaponSlot WeaponSlot);

	bool FireCheck(int32 AmmoCount);

	void FireFX(USoundBase* Sound, FVector Location, USoundAttenuation* AttenuationSettings, USoundConcurrency* Concurrency);

	void EmptyFX(USoundBase* Sound);

	void MuzzleVFX(UNiagaraSystem* NiagaraSystem, USceneComponent* AttachToComponent);

	void FireMontage(UAnimMontage* FireAnim);

	bool CheckAmmo();

	float ReloadMontage(UAnimMontage* ReloadAnim);

	void ReloadCheck();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	USkeletalMesh* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AWeapon_Base> MasterWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Animation")
	EAnimationState AnimationState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bIsDryAmmo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	UUserWidget* CrosshairWidget;

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

	AFFCharacter* CharacterRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	UWeaponData* RifleData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	UWeaponData* PistolData;
};
