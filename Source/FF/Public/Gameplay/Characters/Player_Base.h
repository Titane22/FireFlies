// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Characters/FFCharacter.h"
#include "Player_Base.generated.h"

class APC_Base;
class AMasterWeapon;
class UInputAction;
class UWeaponData;
struct FInputActionValue;

/**
 * 
 */
UCLASS()
class FF_API APlayer_Base : public AFFCharacter
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchWeaponsAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchPrimaryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchHandgunAction;

	UPROPERTY(BlueprintReadWrite, Category = "Equipment")
	AMasterWeapon* CurrentWeapon;
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	class UW_DynamicWeaponHUD* CurrentWeaponUI;
	
protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	 
	virtual void BeginPlay();
	
	virtual void SwitchWeapon(EEquipmentSlot Slot) override;

	virtual void SwitchToPrimaryWeapon() override;
	virtual void SwitchToHandgunWeapon() override;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void UpdateWeaponUI(UWeaponData* WeaponData);

	void ShootFire(const FInputActionValue& Value);

	void Reload();
	
	bool CanFire();

	void HandleFiring();
	void ReadyToFire(AMasterWeapon* MasterWeapon, UWeaponData* CurrentWeaponDataAsset);

	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void Interact_Started();
	void Interact_Triggered(); // Hold completed (for Enhanced Input Hold Trigger)
	void Interact_Completed(); // Key released
	void Interact();

private:
	bool bInteractHoldTriggered = false;
	float InteractStartTime = 0.0f;
	FTimerHandle InteractHoldTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float HoldThreshold = 1.0f; // Hold time in seconds

	void OnInteractHoldCompleted();

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Camera")
	class UCameraComponent* GetFollowCamera() const;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Camera")
	class USpringArmComponent* GetCameraBoom() const;

	APlayerCameraManager* GetPlayerCameraManager() const;
	
	bool CanSwitchWeapon();
};
