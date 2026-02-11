// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "W_MasterHUD.generated.h"

class APlayer_Base;
class UImage;
class UTextBlock;

/**
 * HUD State enum for Master HUD
 */
UENUM(BlueprintType)
enum class EHUDState : uint8
{
	Armed		UMETA(DisplayName = "Armed"),
	Unarmed		UMETA(DisplayName = "Unarmed"),
	Interacting	UMETA(DisplayName = "Interacting")
};

/**
 * Master HUD - Single unified widget managing all player HUD elements
 *
 * Structure:
 * ├── WeaponPanel (무기 정보 - Armed 상태에서 표시)
 * ├── CrosshairPanel (조준점 - 항상 표시, 스타일만 변경)
 * └── InteractionPanel (상호작용 프로그레스 - Interacting 상태에서 표시)
 */
UCLASS()
class FF_API UW_MasterHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	//==========================================================================
	// State Management
	//==========================================================================

	/** Set HUD state and update panel visibility */
	UFUNCTION(BlueprintCallable, Category = "HUD|State")
	void SetHUDState(EHUDState NewState);

	/** Get current HUD state */
	UFUNCTION(BlueprintPure, Category = "HUD|State")
	EHUDState GetHUDState() const { return CurrentState; }

	//==========================================================================
	// Weapon Panel (Armed State)
	//==========================================================================

	/** Set weapon data (icon, name, ammo) */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD|Weapon")
	void SetWeaponData(UTexture2D* Texture, const FString& WeaponName, int32 MaxAmmo, int32 CurrentClip);

	/** Update ammo count display */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD|Weapon")
	void UpdateAmmoCount(int32 MaxAmmo, int32 CurrentClip);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Weapon", meta = (DisplayName = "ParseAmmoCount"))
	FString ParseAmmoCount(int32 AmmoCount);
	
	/** Show weapon panel */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD|Weapon")
	void ShowWeaponPanel();

	/** Hide weapon panel */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD|Weapon")
	void HideWeaponPanel();

	//==========================================================================
	// Crosshair Panel (Always visible, style changes)
	//==========================================================================

	/** Set crosshair style based on state */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD|Crosshair")
	void SetCrosshairStyle(EHUDState State);

	/** Set crosshair color */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD|Crosshair")
	void SetCrosshairColor(FLinearColor Color);

	/** Show/Hide crosshair */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD|Crosshair")
	void SetCrosshairVisible(bool bVisible);

	//==========================================================================
	// Hit Marker
	//==========================================================================

	/** Show hit marker feedback (white flash for hit, red for kill) - implement in Blueprint */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD|HitMarker")
	void ShowHitMarker(bool bIsKill);

	//==========================================================================
	// Interaction Panel (Interacting State)
	//==========================================================================

	/** Show interaction panel with prompt text */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD|Interaction")
	void ShowInteractionPanel(const FText& PromptText);

	/** Hide interaction panel */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD|Interaction")
	void HideInteractionPanel();

	/** Update interaction progress (0.0 ~ 1.0) */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD|Interaction")
	void UpdateInteractionProgress(float Progress);

	/** Called when interaction completes */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD|Interaction")
	void OnInteractionCompleted();

	/** Called when interaction is cancelled */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD|Interaction")
	void OnInteractionCancelled();

	//==========================================================================
	// State Transition Events (for Blueprint animations)
	//==========================================================================

	/** Called when transitioning to Armed state */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|State")
	void OnEnterArmedState();

	/** Called when transitioning to Unarmed state */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|State")
	void OnEnterUnarmedState();

	/** Called when transitioning to Interacting state */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD|State")
	void OnEnterInteractingState();

public:
	/** Reference to the player character */
	UPROPERTY(BlueprintReadWrite, Category = "Player")
	APlayer_Base* CharacterRef;

protected:
	/** Current HUD state */
	UPROPERTY(BlueprintReadOnly, Category = "HUD|State")
	EHUDState CurrentState = EHUDState::Unarmed;

	/** Previous state (for transition logic) */
	UPROPERTY(BlueprintReadOnly, Category = "HUD|State")
	EHUDState PreviousState = EHUDState::Unarmed;

	//==========================================================================
	// Widget Bindings (Optional - for C++ access)
	//==========================================================================

	/** Weapon icon image */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Components")
	UImage* WeaponIconImage;

	/** Crosshair image */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Components")
	UImage* CrosshairImage;

	/** Interaction prompt text */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Components")
	UTextBlock* InteractionPromptText;
};
