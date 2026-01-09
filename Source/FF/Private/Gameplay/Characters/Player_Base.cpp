// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Characters/Player_Base.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

void APlayer_Base::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		/*EnhancedInputComponent->BindAction(SwitchWeaponsAction, ETriggerEvent::Triggered, this, &AFFCharacter::SwitchWeapons);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AFFCharacter::Reload);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AFFCharacter::ShootFire);*/
	}
}
