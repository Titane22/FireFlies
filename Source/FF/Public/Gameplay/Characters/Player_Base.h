// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Characters/FFCharacter.h"
#include "Player_Base.generated.h"

class UConsumableData;
class APC_Base;
class AMasterWeapon;
class UInputAction;
class UWeaponData;
class UW_MasterHUD;
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
	UInputAction* InventoryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FlashOnAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchWeaponsAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchPrimaryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchHandgunAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchMeleeAction;
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	UW_MasterHUD* MasterHUD;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Data")
	UAnimMontage* OpenInventoryAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Data")
	UAnimMontage* CloseInventoryAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Data")
	UAnimMontage* InventoryLoopAnimMontage;
protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	 
	virtual void BeginPlay();
	
	virtual void SwitchWeapon(EEquipmentSlot Slot) override;

	virtual void SwitchToPrimaryWeapon() override;
	virtual void SwitchToHandgunWeapon() override;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void UpdateWeaponUI(UWeaponData* WeaponData);

	void OnAttackStarted(const FInputActionValue& Value);
	void OnAttackHeld(const FInputActionValue& Value);
	void OnAttackReleased(const FInputActionValue& Value);

	void Reload();

	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void Interact_Started();
	void Interact_Completed(); // Key released
	void Interact();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void CloseInventory(UAnimInstance* AnimInst);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void Inventory();

	UFUNCTION()
	void OnOpenMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnCloseMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void HandleConsumableUsed(UConsumableData* ConsumableData, const FItemSlot& Slot);
private:
	bool bInteractHoldTriggered = false;
	float InteractStartTime = 0.0f;
	FTimerHandle InteractHoldTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Interaction")
	float HoldThreshold = 1.0f; // Hold time in seconds

	void OnInteractHoldCompleted();

	//==========================================================================
	// Interaction UI
	//==========================================================================

	/** Timer for updating interaction UI progress at 60 FPS */
	FTimerHandle InteractionUIUpdateHandle;

	/** Cached hold duration for current interaction */
	float CurrentInteractionHoldDuration = 1.0f;

	/** Update interaction progress bar during hold interaction */
	void UpdateInteractionUIProgress();

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Camera")
	class UCameraComponent* GetFollowCamera() const;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Camera")
	class USpringArmComponent* GetCameraBoom() const;

	APlayerCameraManager* GetPlayerCameraManager() const;
	
	bool CanSwitchWeapon();

	void OpenLootingUI(UInventorySystem* ContainerInven);
};
