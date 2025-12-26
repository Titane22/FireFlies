// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Equipments/Weapons/Weapon_Base.h"
#include "Gameplay/Characters/FFCharacter.h"
#include "Gameplay/Components/WeaponSystem.h"
#include "Gameplay/Data/WeaponData.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AWeapon_Base::AWeapon_Base()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent = DefaultSceneRoot;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMesh");
	WeaponMesh->SetupAttachment(DefaultSceneRoot);

	WeaponSystem = CreateDefaultSubobject<UWeaponSystem>("WeaponSystem");

	AnimationState = EAnimationState::Unarmed;
	bReloading = false;
	bAutoReload = false;

	WeaponSystem->MasterWeapon = this->GetClass();
	WeaponSystem->AnimationState = EAnimationState::Unarmed;
	WeaponSystem->Weapon_Details = {
		FWeapon_Data{
			/* CurrentAmmo */ 32,
			/* MaxAmmo */ 90,
			/* ClipAmmo */ 32,
			/* DifferentAmmo */ 90,
			/* Ammo_Count */ 1,
			/* ShortGun_Trace */ false
		}
	};

	WeaponSystem->bIsDryAmmo = false;
	WeaponSystem->CrosshairWidget = nullptr;
}

void AWeapon_Base::Fire()
{
}

void AWeapon_Base::Reload()
{
}

// Called when the game starts or when spawned
void AWeapon_Base::BeginPlay()
{
	Super::BeginPlay();

}

bool AWeapon_Base::ApplyHit(const FHitResult HitResult, bool& bValidHit)
{
	AActor* HitActor = HitResult.GetActor();

	if (!HitActor)
	{
		bValidHit = false;
		return false;
	}

	// TODO: Find Attached Hurtbox from HitActor 
	UActorComponent* HurtboxComp = nullptr;
	if (!HurtboxComp)
	{
		AFFCharacter* OwnerRef = Cast<AFFCharacter>(HitActor);
		if (!OwnerRef)
		{
			UGameplayStatics::ApplyDamage
			(
				HitActor,
				WeaponData->Damage,
				GetInstigatorController(),
				this,
				nullptr
			);
			bValidHit = false;
			return false;
		}

		if (!OwnerRef->bisDead)
		{
			// TODO: OwnerRef->GetHealthComponent()->ApplyDamage(WeaponData->Damage);
			if (HitMarkerSound)
			{
				UGameplayStatics::PlaySound2D(
					this,           // WorldContextObject
					HitMarkerSound, // Sound
					1.0f,          // Volume Multiplier
					1.0f,          // Pitch Multiplier
					0.0f,          // Start Time
					nullptr,       // Concurrency Settings
					nullptr,       // Owning Actor
					true          // Is UI Sound
				);
			}
			bValidHit = true;
			return bValidHit;
		}

		// 대상이 이미 죽었을 경우
		bValidHit = false;
		return false;
	}
	else
	{
		if (HitMarkerSound)
		{
			UGameplayStatics::PlaySound2D(
				this,           // WorldContextObject
				HitMarkerSound, // Sound
				1.0f,          // Volume Multiplier
				1.0f,          // Pitch Multiplier
				0.0f,          // Start Time
				nullptr,       // Concurrency Settings
				nullptr,       // Owning Actor
				true          // Is UI Sound
			);
		}
		bValidHit = true;
		return bValidHit;
	}
}


