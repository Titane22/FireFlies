// Fill out your copyright notice in the Description page of Project Settings.


#include "Presentations/HUD/PlayerHUD.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/Data/WeaponData.h"
#include "Presentations/HUD/W_DynamicWeaponHUD.h"

UW_DynamicWeaponHUD* APlayerHUD::ShowWeaponUI(UWeaponData* WeaponData, int32 MaxAmmo, int32 CurrentAmmo)
{
	if (!WeaponData || !WeaponData->WeaponUI)
	{
		UE_LOG(LogTemp, Error, TEXT("[ShowWeaponUI] Invalid WeaponData or WeaponUI"));
		return nullptr;
	}

	HideWeaponUI();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("[ShowWeaponUI] No PlayerController found"));
		return nullptr;
	}

	UUserWidget* WeaponUIWidget = CreateWidget<UUserWidget>(PC, WeaponData->WeaponUI);
	if (!WeaponUIWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[ShowWeaponUI] Failed to create widget"));
		return nullptr;
	}

	CurrentWeaponUI = Cast<UW_DynamicWeaponHUD>(WeaponUIWidget);
	if (!CurrentWeaponUI)
	{
		UE_LOG(LogTemp, Error, TEXT("[ShowWeaponUI] Failed to cast to UW_DynamicWeaponHUD"));
		return nullptr;
	}

	CurrentWeaponUI->AddToViewport();
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString::Printf(TEXT("Called: %s"), *CurrentWeaponUI->GetName()));
	
	return CurrentWeaponUI;
}

void APlayerHUD::HideWeaponUI()
{
	if (CurrentWeaponUI)
	{
		CurrentWeaponUI->RemoveFromParent();
		CurrentWeaponUI = nullptr;
	}
}

void APlayerHUD::UpdateWeaponAmmo(int32 MaxAmmo, int32 CurrentAmmo)
{
	if (CurrentWeaponUI)
	{
		CurrentWeaponUI->UpdateAmmoCount(MaxAmmo, CurrentAmmo);
	}
}

void APlayerHUD::SetWeaponDataOnHUD(UTexture2D* Texture, const FString& WeaponName, int32 MaxAmmo, int32 CurrentClip)
{
	if (CurrentWeaponUI)
	{
		CurrentWeaponUI->SetWeaponData(Texture, WeaponName, MaxAmmo, CurrentClip);
	}
}
