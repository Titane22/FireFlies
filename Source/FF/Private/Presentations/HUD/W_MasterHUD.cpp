// Fill out your copyright notice in the Description page of Project Settings.

#include "Presentations/HUD/W_MasterHUD.h"

void UW_MasterHUD::SetHUDState(EHUDState NewState)
{
	if (CurrentState == NewState)
		return;

	PreviousState = CurrentState;
	CurrentState = NewState;

	// Update panel visibility based on state
	switch (NewState)
	{
	case EHUDState::Armed:
		ShowWeaponPanel();
		HideInteractionPanel();
		SetCrosshairStyle(EHUDState::Armed);
		OnEnterArmedState();
		break;

	case EHUDState::Unarmed:
		HideWeaponPanel();
		HideInteractionPanel();
		SetCrosshairStyle(EHUDState::Unarmed);
		OnEnterUnarmedState();
		break;

	case EHUDState::Interacting:
		HideWeaponPanel();
		// InteractionPanel is shown via ShowInteractionPanel() with prompt text
		SetCrosshairStyle(EHUDState::Interacting);
		OnEnterInteractingState();
		break;
	}

	UE_LOG(LogTemp, Log, TEXT("[MasterHUD] State changed: %d -> %d"), (int32)PreviousState, (int32)CurrentState);
}
