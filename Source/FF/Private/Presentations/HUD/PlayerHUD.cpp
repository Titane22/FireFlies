// Fill out your copyright notice in the Description page of Project Settings.

#include "Presentations/HUD/PlayerHUD.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/Data/WeaponData.h"
#include "Presentations/HUD/W_MasterHUD.h"

void APlayerHUD::BeginPlay()
{
	Super::BeginPlay();

	// Auto-create Master HUD on begin play
	ShowMasterHUD();
}

//==============================================================================
// Master HUD Management
//==============================================================================

UW_MasterHUD* APlayerHUD::ShowMasterHUD()
{
	if (MasterHUD)
		return MasterHUD;

	if (!MasterHUDClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerHUD] MasterHUDClass is not set"));
		return nullptr;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerHUD] No PlayerController found"));
		return nullptr;
	}

	UUserWidget* Widget = CreateWidget<UUserWidget>(PC, MasterHUDClass);
	if (!Widget)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerHUD] Failed to create MasterHUD widget"));
		return nullptr;
	}

	MasterHUD = Cast<UW_MasterHUD>(Widget);
	if (!MasterHUD)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerHUD] Failed to cast to UW_MasterHUD"));
		return nullptr;
	}

	MasterHUD->AddToViewport();
	UE_LOG(LogTemp, Log, TEXT("[PlayerHUD] MasterHUD created and added to viewport"));

	return MasterHUD;
}

void APlayerHUD::HideMasterHUD()
{
	if (MasterHUD)
	{
		MasterHUD->RemoveFromParent();
		MasterHUD = nullptr;
		UE_LOG(LogTemp, Log, TEXT("[PlayerHUD] MasterHUD removed"));
	}
}

//==============================================================================
// State Management
//==============================================================================

void APlayerHUD::SetHUDState(EHUDState NewState)
{
	if (MasterHUD)
	{
		MasterHUD->SetHUDState(NewState);
	}
}

EHUDState APlayerHUD::GetHUDState() const
{
	if (MasterHUD)
	{
		return MasterHUD->GetHUDState();
	}
	return EHUDState::Unarmed;
}

//==============================================================================
// Weapon Functions
//==============================================================================

UW_MasterHUD* APlayerHUD::ShowWeaponUI(UWeaponData* WeaponData, int32 MaxAmmo, int32 CurrentAmmo)
{
	if (!WeaponData)
	{
		UE_LOG(LogTemp, Error, TEXT("[ShowWeaponUI] Invalid WeaponData"));
		return nullptr;
	}

	// Ensure MasterHUD exists
	if (!MasterHUD)
	{
		ShowMasterHUD();
	}

	if (!MasterHUD)
	{
		UE_LOG(LogTemp, Error, TEXT("[ShowWeaponUI] Failed to create MasterHUD"));
		return nullptr;
	}

	// Set to Armed state
	MasterHUD->SetHUDState(EHUDState::Armed);

	return MasterHUD;
}

void APlayerHUD::HideWeaponUI()
{
	if (MasterHUD)
	{
		MasterHUD->SetHUDState(EHUDState::Unarmed);
	}
}

void APlayerHUD::UpdateWeaponAmmo(int32 MaxAmmo, int32 CurrentAmmo)
{
	if (MasterHUD)
	{
		MasterHUD->UpdateAmmoCount(MaxAmmo, CurrentAmmo);
	}
}

void APlayerHUD::SetWeaponDataOnHUD(UTexture2D* Texture, const FString& WeaponName, int32 MaxAmmo, int32 CurrentClip)
{
	if (MasterHUD)
	{
		MasterHUD->SetWeaponData(Texture, WeaponName, MaxAmmo, CurrentClip);
	}
}

//==============================================================================
// Hit Marker
//==============================================================================

void APlayerHUD::ShowHitMarker(bool bIsKill)
{
	if (MasterHUD)
	{
		MasterHUD->ShowHitMarker(bIsKill);
	}
}

//==============================================================================
// Interaction Functions
//==============================================================================

void APlayerHUD::StartInteractionUI(const FText& PromptText)
{
	if (bInteractionUIActive)
		return;

	bInteractionUIActive = true;

	// Remember current state to restore later
	if (MasterHUD)
	{
		StateBeforeInteraction = MasterHUD->GetHUDState();
	}

	// Ensure MasterHUD exists
	if (!MasterHUD)
	{
		ShowMasterHUD();
	}

	if (MasterHUD)
	{
		// Transition to Interacting state
		MasterHUD->SetHUDState(EHUDState::Interacting);
		MasterHUD->ShowInteractionPanel(PromptText);
	}

	UE_LOG(LogTemp, Log, TEXT("[StartInteractionUI] Interaction UI started"));
}

void APlayerHUD::UpdateInteractionUI(float Progress)
{
	if (!bInteractionUIActive)
		return;

	if (MasterHUD)
	{
		MasterHUD->UpdateInteractionProgress(Progress);
	}
}

void APlayerHUD::CompleteInteractionUI()
{
	if (!bInteractionUIActive)
		return;

	if (MasterHUD)
	{
		MasterHUD->OnInteractionCompleted();
		MasterHUD->HideInteractionPanel();
	}

	// Restore previous state
	RestorePreviousState();

	bInteractionUIActive = false;
	UE_LOG(LogTemp, Log, TEXT("[CompleteInteractionUI] Interaction completed, restored to previous state"));
}

void APlayerHUD::CancelInteractionUI()
{
	if (!bInteractionUIActive)
		return;

	if (MasterHUD)
	{
		MasterHUD->OnInteractionCancelled();
		MasterHUD->HideInteractionPanel();
	}

	// Restore previous state
	RestorePreviousState();

	bInteractionUIActive = false;
	UE_LOG(LogTemp, Log, TEXT("[CancelInteractionUI] Interaction cancelled, restored to previous state"));
}

void APlayerHUD::RestorePreviousState()
{
	if (MasterHUD)
	{
		MasterHUD->SetHUDState(StateBeforeInteraction);
	}
}
