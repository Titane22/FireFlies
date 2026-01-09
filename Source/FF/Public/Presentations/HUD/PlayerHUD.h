// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"

class UW_DynamicWeaponHUD;
class UWeaponData;
class AMasterWeapon;
/**
 * 
 */
UCLASS()
class FF_API APlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "HUD|Weapon")
	UW_DynamicWeaponHUD* ShowWeaponUI(
		UWeaponData* WeaponData,
		int32 MaxAmmo,
		int32 CurrentAmmo);

	UFUNCTION(BlueprintCallable, Category = "HUD|Weapon")
	void HideWeaponUI();

	UFUNCTION(BlueprintCallable, Category = "HUD|Weapon")
	void UpdateWeaponAmmo(int32 MaxAmmo, int32 CurrentAmmo);

	UFUNCTION(BlueprintCallable, Category = "HUD|Weapon")
	void SetWeaponDataOnHUD(UTexture2D* Texture, const FString& WeaponName, int32 MaxAmmo, int32 CurrentClip);

	FORCEINLINE UW_DynamicWeaponHUD* GetCurrentWeaponUI() const { return CurrentWeaponUI; }
protected:
	UPROPERTY()
	UW_DynamicWeaponHUD* CurrentWeaponUI;
};
