// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Gameplay/Data/ItemData.h"
#include "FFCharacter.generated.h"

class UHurtbox;
class UHealthSystem;
class UInventorySystem;
class UEquipmentSystem;
class UPhysicalAnimationComponent;
struct FInputActionValue;

UCLASS()
class FF_API AFFCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFFCharacter();
	
	UPhysicalAnimationComponent* GetPAC() const { return PAC; }

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void FlashOnOff();

protected:
	virtual void BeginPlay();

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void SwitchWeapon(EEquipmentSlot Slot);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void SwitchToPrimaryWeapon();

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void SwitchToHandgunWeapon();
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TMap<EEquipmentSlot, UChildActorComponent*> EquippedChilds;

	// Weapon Child Actor Components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* PrimaryChild;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* HandgunChild;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	UChildActorComponent* FlashlightChild;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Primary;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Handgun;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UPhysicalAnimationComponent* PAC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UEquipmentSystem* EquipmentSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UInventorySystem* InventorySystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UHealthSystem* HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UHurtbox* Hurtbox;

	bool bCanSwitchWeapon = true;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bFiring;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Equipment|Action")
	bool bIsAiming;

	bool bCanFire = true;

	bool IsCrouch;

	bool IsSprint;

	bool bIsDodging;
};
