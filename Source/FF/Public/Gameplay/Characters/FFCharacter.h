// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Gameplay/Library/AnimationState.h"
#include "Gameplay/Data/WeaponData.h"
#include "FFCharacter.generated.h"

class AWeapon_Base;
class UWeaponSystem;
struct FInputActionValue;

UCLASS()
class FF_API AFFCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFFCharacter();
	
	/** Primary Child Socket*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* PrimaryChild;

	/** Handgun Child Socket*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* HandgunChild;

	// Movement States
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsCrouching;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsSprinting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsSliding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsDodging;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsJumping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bisDead;

	// State Variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	ELandState CurrentLandState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Animation")
	EAnimationState CurrentAnimationState;

	AWeapon_Base* CurrentWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bIsAim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bIsPistolEquip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bIsPrimaryEquip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bIsWeaponEquip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bCanFire = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool IsAttacking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bCanSwitchWeapon = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bFiring;

	// Input Action 함수들 (public)
	UFUNCTION(BlueprintCallable, Category = "Weapon|InputAction")
	virtual void SwitchWeapons();

	UFUNCTION(BlueprintCallable, Category = "Weapon|InputAction")
	virtual void SwitchToPrimaryWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapon|InputAction")
	virtual void SwitchToHandgunWeapon();

	UFUNCTION()
	void ShootFire(const FInputActionValue& Value);

	UFUNCTION()
	void Aim(const FInputActionValue& Value);

	UFUNCTION()
	void Reload();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void ReadyToFire(AWeapon_Base* MasterWeapon, UWeaponData* CurrentWeaponDataAsset);

	void StopFire();

	void HandleFiring();

	bool CanFire();

	bool CanSwitchWeapon();
	
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void Die();

	UFUNCTION()
	void StartRagdoll();
	
protected:
	/** Primary Socket*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Primary;

	/** Hand-Gun Socket*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Handgun;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UWeaponSystem* WeaponSystem;

};
