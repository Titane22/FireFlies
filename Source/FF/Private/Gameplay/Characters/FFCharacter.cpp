// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Characters/FFCharacter.h"
#include "Gameplay/Data/WeaponData.h"
#include "Gameplay/Equipments/Weapons/Weapon_Base.h"
#include "Gameplay/Components/WeaponSystem.h"
#include "Components/CapsuleComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AFFCharacter::AFFCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Primary = CreateDefaultSubobject<USceneComponent>(TEXT("Primary"));
	Primary->SetupAttachment(RootComponent);
	
	PrimaryChild = CreateDefaultSubobject<UChildActorComponent>(TEXT("PrimaryChild"));
	PrimaryChild->SetupAttachment(Primary);
	
	Handgun = CreateDefaultSubobject<USceneComponent>(TEXT("Handgun"));
	Handgun->SetupAttachment(RootComponent);
	
	HandgunChild = CreateDefaultSubobject<UChildActorComponent>(TEXT("HandgunChild"));
	HandgunChild->SetupAttachment(Handgun);
	
	WeaponSystem = CreateDefaultSubobject<UWeaponSystem>(TEXT("WeaponSystem"));
}

// Called when the game starts or when spawned
void AFFCharacter::BeginPlay()
{
	Super::BeginPlay();

	WeaponSystem->CharacterRef = this;
}

void AFFCharacter::SwitchWeapons()
{
}

void AFFCharacter::Die()
{
	if (bisDead)
		return;

	FVector InteractionActorLocation = GetCapsuleComponent()->GetComponentLocation();

	FTransform InteractionSpawnTransform;
	InteractionSpawnTransform.SetLocation(FVector(InteractionActorLocation.X, InteractionActorLocation.Y, InteractionActorLocation.Z + 20.0f));
	InteractionSpawnTransform.SetRotation(FQuat::Identity);
	InteractionSpawnTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));

	bisDead = true;

	// TODO: Pickup
	// if (AWeapon_Base* PrimaryWeapon = Cast<AWeapon_Base>(PrimaryChild->GetChildActor()))
	// {
	// 	GetWorld()->SpawnActor<AActor>(PrimaryWeapon->WeaponPickupClass, InteractionSpawnTransform);
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Die()::PrimaryWeapon Is Null"));
	// }
	// if (AWeapon_Base* SecondaryWeapon = Cast<AWeapon_Base>(HandgunChild->GetChildActor()))
	// {
	// 	GetWorld()->SpawnActor<AActor>(SecondaryWeapon->WeaponPickupClass, InteractionSpawnTransform);
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Die()::SecondaryWeapon Is Null"));
	// }

	// 1. Disable capsule collision and enable ragdoll physics
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);

	// 2. Disable character movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	// 3. Detach controller from character
	if (AController* CharacterController = GetController())
	{
		CharacterController->UnPossess();
	}
}

void AFFCharacter::StartRagdoll()
{
	// Disable Character Movement
	GetCharacterMovement()->DisableMovement();

	// Enable Ragdoll Physics on body mesh
	GetMesh()->SetSimulatePhysics(true);

	// Disable Capsule Component Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Hide Weapons
	PrimaryChild->SetVisibility(false, true);
	HandgunChild->SetVisibility(false, true);
}

void AFFCharacter::SwitchToPrimaryWeapon()
{
	if (!CanSwitchWeapon() || bIsPrimaryEquip)
		return;

	bCanSwitchWeapon = false;
	bIsPistolEquip = false;

	// Handgun을 허리에 부착 (Unequip)
	WeaponSystem->SetWeaponState(
		WeaponSystem->PistolData->WeaponClass,
		EAnimationState::Pistol,
		EWeaponState::Unequip,
		FName(""),
		FName("PistolHost_Socket"),
		EWeaponSlot::Handgun
	);

	bIsPrimaryEquip = true;

	// Primary 무기를 손에 장착 (Equip)
	WeaponSystem->SetWeaponState(
		WeaponSystem->RifleData->WeaponClass,
		EAnimationState::RifleShotgun,
		EWeaponState::Equip,
		FName("Rifle_Socket"),
		FName(""),
		EWeaponSlot::Primary
	);
	{
		AWeapon_Base* MasterWeapon = Cast<AWeapon_Base>(HandgunChild->GetChildActor());
	}
	// Play Montage with Delay
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimInstance is NULL"));
		return;
	}

	if (AnimInstance)
	{
		UAnimMontage* RifleEquipMontage = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr, TEXT("/Game/ThirdPerson/Blueprints/Animation/Weapons/Rifle/Montages/MM_Rifle_Equip1")));
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("RifleEquipMontage %s"));
		// Play Montage
		AnimInstance->Montage_Play(RifleEquipMontage, 1.0f);

		// Set Montage End Delegate
		FOnMontageEnded CompleteDelegate;
		CompleteDelegate.BindUObject(this, &AFFCharacter::OnMontageEnded);
		AnimInstance->Montage_SetEndDelegate(CompleteDelegate, RifleEquipMontage);
	}
}

void AFFCharacter::SwitchToHandgunWeapon()
{
	if (!CanSwitchWeapon() || bIsPistolEquip)
		return;

	bCanSwitchWeapon = false;
	bIsPrimaryEquip = false;

	// Primary 무기를 등에 부착 (Unequip)
	WeaponSystem->SetWeaponState(
		WeaponSystem->RifleData->WeaponClass,
		EAnimationState::RifleShotgun,
		EWeaponState::Unequip,
		FName(""),
		FName("RifleHost_Socket"),
		EWeaponSlot::Primary
	);

	bIsPistolEquip = true;

	// Handgun을 손에 장착 (Equip)
	WeaponSystem->SetWeaponState(
		WeaponSystem->PistolData->WeaponClass,
		EAnimationState::Pistol,
		EWeaponState::Equip,
		FName("Pistol_Socket"),
		FName(""),
		EWeaponSlot::Handgun
	);

	// Play Montage with Delay
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimInstance is NULL"));
		return;
	}

	if (AnimInstance)
	{
		UAnimMontage* PistolEquipMontage = Cast<UAnimMontage>(StaticLoadObject(UAnimMontage::StaticClass(), nullptr, TEXT("/Game/ThirdPerson/Blueprints/Animation/Weapons/Pistol/Montages/MM_Pistol_Equip2")));
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("RifleEquipMontage %s"));
		// Play Montage
		AnimInstance->Montage_Play(PistolEquipMontage, 1.0f);

		// Set Montage End Delegate
		FOnMontageEnded CompleteDelegate;
		CompleteDelegate.BindUObject(this, &AFFCharacter::OnMontageEnded);
		AnimInstance->Montage_SetEndDelegate(CompleteDelegate, PistolEquipMontage);
	}
}

void AFFCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// After Montage Ended
	if (!bInterrupted)
	{
		bCanSwitchWeapon = true;
	}
}

void AFFCharacter::ShootFire(const FInputActionValue& Value)
{
	if (!bIsAim)
		return;
	bFiring = Value.Get<bool>();
	
	HandleFiring();
}

void AFFCharacter::Aim(const FInputActionValue& Value)
{
	if (!bIsPrimaryEquip && !bIsPistolEquip)
		return;
	bIsAim = Value.Get<bool>();

}

void AFFCharacter::Reload()
{
	AWeapon_Base* MasterWeapon = nullptr;  // nullptr로 초기화

	if (bIsPrimaryEquip)
	{
		MasterWeapon = Cast<AWeapon_Base>(PrimaryChild->GetChildActor());
	}
	else if (bIsPistolEquip)
	{
		MasterWeapon = Cast<AWeapon_Base>(HandgunChild->GetChildActor());
	}
	else
	{
		return; // 무기를 들고 있지 않은 경우
	}

	if (!MasterWeapon)
		return;

	MasterWeapon->Reload();
}

void AFFCharacter::StopFire()
{
	bFiring = false;
}

void AFFCharacter::HandleFiring()
{
	if (!bFiring)
		return;

	if (!bCanFire)
		return;

	if (!CanFire())
		return;

	if (bIsPrimaryEquip)
	{
		AWeapon_Base* MasterWeapon = Cast<AWeapon_Base>(PrimaryChild->GetChildActor());
		UWeaponData* CurrentWeaponDataAsset = WeaponSystem->RifleData;
		ReadyToFire(MasterWeapon, CurrentWeaponDataAsset);
	}
	else if(bIsPistolEquip)
	{
		AWeapon_Base* MasterWeapon = Cast<AWeapon_Base>(HandgunChild->GetChildActor());
		UWeaponData* CurrentWeaponDataAsset = WeaponSystem->PistolData;
		ReadyToFire(MasterWeapon, CurrentWeaponDataAsset);
	}
}

bool AFFCharacter::CanFire()
{
	bool bCanShoot = !bIsSprinting && !bIsDodging && CanJump() && bCanSwitchWeapon;
	if (bCanShoot || bIsCrouching)
		return true;
	else
		return false;
}

bool AFFCharacter::CanSwitchWeapon()
{
	return bCanSwitchWeapon;
}

void AFFCharacter::ReadyToFire(AWeapon_Base* MasterWeapon, UWeaponData* CurrentWeaponDataAsset)
{
	if (!MasterWeapon || !CurrentWeaponDataAsset)
		return;
	
	bCanFire = false;
	MasterWeapon->Fire();

	// Process next shot based on fire mode
	float FireDelay = CurrentWeaponDataAsset->FireRate;
	EFireMode CurrentFireMode = CurrentWeaponDataAsset->FireMode;
	FTimerHandle TimerHandle;

	switch (CurrentFireMode)
	{
	case EFireMode::FullAuto:
		// Auto-fire is scheduled with a timer
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[this]()
			{
				bCanFire = true;
				HandleFiring(); // Recursive firing process
				//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Fire!!!!!!!!!!!!!!!!!!!!!!!!!"));
			},
			FireDelay,
			false
		);
		break;
	default:
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[this]()
			{
				bCanFire = true;
			},
			FireDelay,
			false
		);
		break;
	}
}

// Called every frame
void AFFCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AFFCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

