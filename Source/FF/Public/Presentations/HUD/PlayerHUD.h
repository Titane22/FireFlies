// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Presentations/HUD/W_MasterHUD.h"
#include "PlayerHUD.generated.h"

class UW_MasterHUD;
class UWeaponData;

/**
 * Player HUD - manages the single Master HUD widget
 */
UCLASS()
class FF_API APlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	//==========================================================================
	// Master HUD Management
	//==========================================================================

	/** Initialize and show the Master HUD */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	UW_MasterHUD* ShowMasterHUD();

	/** Hide and destroy the Master HUD */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void HideMasterHUD();

	/** Get current Master HUD */
	UFUNCTION(BlueprintPure, Category = "HUD")
	UW_MasterHUD* GetMasterHUD() const { return MasterHUD; }

	//==========================================================================
	// State Management
	//==========================================================================

	/** Set HUD state */
	UFUNCTION(BlueprintCallable, Category = "HUD|State")
	void SetHUDState(EHUDState NewState);

	/** Get current HUD state */
	UFUNCTION(BlueprintPure, Category = "HUD|State")
	EHUDState GetHUDState() const;

	//==========================================================================
	// Weapon Functions (Armed State)
	//==========================================================================

	/** Show weapon UI and set to Armed state */
	UFUNCTION(BlueprintCallable, Category = "HUD|Weapon")
	UW_MasterHUD* ShowWeaponUI(UWeaponData* WeaponData, int32 MaxAmmo, int32 CurrentAmmo);

	/** Hide weapon UI and set to Unarmed state */
	UFUNCTION(BlueprintCallable, Category = "HUD|Weapon")
	void HideWeaponUI();

	/** Update ammo count display */
	UFUNCTION(BlueprintCallable, Category = "HUD|Weapon")
	void UpdateWeaponAmmo(int32 MaxAmmo, int32 CurrentAmmo);

	/** Set weapon data on HUD */
	UFUNCTION(BlueprintCallable, Category = "HUD|Weapon")
	void SetWeaponDataOnHUD(UTexture2D* Texture, const FString& WeaponName, int32 MaxAmmo, int32 CurrentClip);

	//==========================================================================
	// Hit Marker
	//==========================================================================

	/** Show hit marker on HUD (pass-through to MasterHUD) */
	UFUNCTION(BlueprintCallable, Category = "HUD|HitMarker")
	void ShowHitMarker(bool bIsKill);

	//==========================================================================
	// Interaction Functions (Interacting State)
	//==========================================================================

	/** Start interaction UI - transitions to Interacting state */
	UFUNCTION(BlueprintCallable, Category = "HUD|Interaction")
	void StartInteractionUI(const FText& PromptText);

	/** Update interaction progress (0.0 ~ 1.0) */
	UFUNCTION(BlueprintCallable, Category = "HUD|Interaction")
	void UpdateInteractionUI(float Progress);

	/** Complete interaction - returns to previous state */
	UFUNCTION(BlueprintCallable, Category = "HUD|Interaction")
	void CompleteInteractionUI();

	/** Cancel interaction - returns to previous state */
	UFUNCTION(BlueprintCallable, Category = "HUD|Interaction")
	void CancelInteractionUI();

protected:
	virtual void BeginPlay() override;

	/** Restore previous state after interaction */
	void RestorePreviousState();

protected:
	/** Current Master HUD widget instance */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD")
	UW_MasterHUD* MasterHUD;

	/** Master HUD class to spawn (set in Blueprint) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Classes")
	TSubclassOf<UW_MasterHUD> MasterHUDClass;

	/** State before interaction started (to restore after) */
	EHUDState StateBeforeInteraction = EHUDState::Unarmed;

	/** Track if interaction UI is currently active */
	UPROPERTY(BlueprintReadOnly, Category = "HUD|State")
	bool bInteractionUIActive = false;
};
