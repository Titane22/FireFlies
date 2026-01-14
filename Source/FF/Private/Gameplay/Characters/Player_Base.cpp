// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Characters/Player_Base.h"
#include "Gameplay/Items/Equipments/MasterWeapon.h"
#include "Gameplay/Items/EquipmentSystem.h"
#include "Gameplay/Items/InventorySystem.h"
#include "Gameplay/Items/Interaction/Interactor.h"
#include "Gameplay/Interfaces/Interactable.h"
#include "Gameplay/Data/WeaponData.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Gameplay/Characters/PC_Base.h"
#include "Presentations/HUD/PlayerHUD.h"
#include "Presentations/HUD/W_DynamicWeaponHUD.h"

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
		// EnhancedInputComponent->BindAction(SwitchPrimaryAction, ETriggerEvent::Triggered, this, &APlayer_Base::SwitchToPrimaryWeapon);
		// EnhancedInputComponent->BindAction(SwitchHandgunAction, ETriggerEvent::Triggered, this, &APlayer_Base::SwitchToHandgunWeapon);

		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &APlayer_Base::Reload);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &APlayer_Base::ShootFire);

		// Interact actions - Hold support
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APlayer_Base::Interact_Started);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Canceled, this, &APlayer_Base::Interact_Completed);
	}
}

void APlayer_Base::BeginPlay()
{
	Super::BeginPlay();

	FlashlightChild->SetVisibility(false);
}

void APlayer_Base::SwitchWeapon(EEquipmentSlot Slot)
{
	Super::SwitchWeapon(Slot);
	if (!EquipmentSystem || !CanSwitchWeapon() || EquipmentSystem->CurrentEquippedSlot == Slot)
		return;
	if (!EquipmentSystem->IsEquipped(Slot))
	{
		UE_LOG(LogTemp, Warning, TEXT("There is no equipped weapon in slot %d"), (int32)Slot);
		return;
	}

	bCanSwitchWeapon = false;

	EquipmentSystem->SwitchToWeapon(Slot);

	UChildActorComponent* TargetChildActor = EquipmentSystem->GetChildActorForSlot(Slot);
	if (TargetChildActor)
	{
		if (AMasterWeapon* Weapon = Cast<AMasterWeapon>(TargetChildActor->GetChildActor()))
		{
			CurrentWeapon = Weapon;
		}
	}

	// Wait for Child Actor to initialize and update UI
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			if (CurrentWeapon && CurrentWeapon->WeaponData)
			{
				UpdateWeaponUI(CurrentWeapon->WeaponData);
			}
		},
		0.1f,
		false
	);

	// Play Animation Montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CurrentWeapon && CurrentWeapon->WeaponData)
	{
		UAnimMontage* WeaponEquipMontage = CurrentWeapon->WeaponData->WeaponEquipMontage;
		
		if (WeaponEquipMontage)
		{
			AnimInstance->Montage_Play(WeaponEquipMontage, 1.0f);
			
			FOnMontageEnded CompleteDelegate;
			CompleteDelegate.BindUObject(this, &APlayer_Base::OnMontageEnded);
			AnimInstance->Montage_SetEndDelegate(CompleteDelegate, WeaponEquipMontage);
		}
		else
		{
			bCanSwitchWeapon = true;
		}
	}
	else
	{
		bCanSwitchWeapon = true;
	}
}

void APlayer_Base::SwitchToPrimaryWeapon()
{
	SwitchWeapon(EEquipmentSlot::Primary);
}

void APlayer_Base::SwitchToHandgunWeapon()
{
	SwitchWeapon(EEquipmentSlot::Handgun);
}

APlayerCameraManager* APlayer_Base::GetPlayerCameraManager() const
{
	APC_Base* PC = Cast<APC_Base>(GetController());
	if (!PC)
		return nullptr;

	return PC->PlayerCameraManager;
}

bool APlayer_Base::CanSwitchWeapon()
{
	return bCanSwitchWeapon;
}

void APlayer_Base::UpdateWeaponUI(UWeaponData* WeaponData)
{
	if (!IsPlayerControlled() || !WeaponData)
		return;
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
		return;

	APlayerHUD* PlayerHUD = Cast<APlayerHUD>(PC->GetHUD());
	if (!PlayerHUD)
		return;

	AMasterWeapon* Weapon = nullptr;
	if (EquipmentSystem->CurrentEquippedSlot == EEquipmentSlot::Primary)
	{
		Weapon = Cast<AMasterWeapon>(PrimaryChild->GetChildActor());
	}
	else if (EquipmentSystem->CurrentEquippedSlot == EEquipmentSlot::Handgun)
	{
		Weapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor());
	}
	else
	{
		PlayerHUD->HideWeaponUI();
		return;
	}

	if (!Weapon || !Weapon->WeaponSystem)
	{
		PlayerHUD->HideWeaponUI();
		return;
	}
	
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString::Printf(TEXT("Called: %s"), *WeaponData->WeaponUITexture->GetName()));
	
	CurrentWeaponUI = PlayerHUD->ShowWeaponUI(
		WeaponData,
		Weapon->GetMaxAmmo(),
		Weapon->GetCurrentAmmo());

	CurrentWeaponUI->CharacterRef = this;
	
	PlayerHUD->SetWeaponDataOnHUD(
		WeaponData->WeaponUITexture,
		WeaponData->ItemName.ToString(),
		CurrentWeapon->GetMaxAmmo(),
		CurrentWeapon->GetCurrentAmmo()
	);
}

void APlayer_Base::ShootFire(const FInputActionValue& Value)
{
	if (!bIsAiming)
		return;
	
	bFiring = Value.Get<bool>();
	HandleFiring();
}

void APlayer_Base::Reload()
{
	if (!EquipmentSystem)
		return;

	AMasterWeapon* MasterWeapon = nullptr;

	if (EquipmentSystem->CurrentEquippedSlot == EEquipmentSlot::Primary)
	{
		MasterWeapon = Cast<AMasterWeapon>(PrimaryChild->GetChildActor());
	}
	else if (EquipmentSystem->CurrentEquippedSlot == EEquipmentSlot::Handgun)
	{
		MasterWeapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor());
	}
	else
	{
		return;
	}

	if (!MasterWeapon)
		return;

	MasterWeapon->Reload();
}

bool APlayer_Base::CanFire()
{
	bool bCanJumpNow = CanJump();  // 한 번만 호출하고 저장
	bool bCanShoot = !IsSprint && !bIsDodging && bCanJumpNow && bCanSwitchWeapon;
	
	return bCanShoot;
}

void APlayer_Base::HandleFiring()
{
	if (!EquipmentSystem || !bFiring || !bCanFire || !CanFire())
		return;

	if (EquipmentSystem->CurrentEquippedSlot == EEquipmentSlot::Primary)
	{
		AMasterWeapon* MasterWeapon = Cast<AMasterWeapon>(PrimaryChild->GetChildActor());
		UWeaponData* CurrentWeaponDataAsset = MasterWeapon->WeaponData;
		ReadyToFire(MasterWeapon, CurrentWeaponDataAsset);
	}
	else if (EquipmentSystem->CurrentEquippedSlot == EEquipmentSlot::Handgun)
	{
		AMasterWeapon* MasterWeapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor());
		UWeaponData* CurrentWeaponDataAsset = MasterWeapon->WeaponData;
		ReadyToFire(MasterWeapon, CurrentWeaponDataAsset);
	}
}

void APlayer_Base::ReadyToFire(AMasterWeapon* MasterWeapon, UWeaponData* CurrentWeaponDataAsset)
{
	if (!MasterWeapon || !CurrentWeaponDataAsset)
		return;

	bCanFire = false;
	MasterWeapon->Fire();
	
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		APlayerHUD* PlayerHUD = Cast<APlayerHUD>(PC->GetHUD());
		if (PlayerHUD)
		{
			PlayerHUD->UpdateWeaponAmmo(
				MasterWeapon->GetMaxAmmo(),
				MasterWeapon->GetCurrentAmmo());
		}
	}

	float FireDelay = CurrentWeaponDataAsset->FireRate;
	EFireMode CurrentFireMode = CurrentWeaponDataAsset->FireMode;
	FTimerHandle TimerHandle;

	switch (CurrentFireMode)
	{
	case EFireMode::FullAuto:
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[this]()
			{
				bCanFire = true;
				HandleFiring();
			},
			FireDelay,
			false
		);
		break;
	default:
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[this]()
			{
				bCanFire = true;
			},
			FireDelay,
			false
		);
		break;
	}
}

void APlayer_Base::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		bCanSwitchWeapon = true;
	}
}

void APlayer_Base::Interact_Started()
{
	// Reset hold flag when starting interaction
	bInteractHoldTriggered = false;

	// Record the start time for hold detection
	InteractStartTime = GetWorld()->GetTimeSeconds();

	// Start timer for hold detection
	GetWorld()->GetTimerManager().SetTimer(
		InteractHoldTimerHandle,
		this,
		&APlayer_Base::OnInteractHoldCompleted,
		HoldThreshold,
		false
	);

	UE_LOG(LogTemp, Log, TEXT("Interact_Started - Timer started for %.2f seconds"), HoldThreshold);
}

void APlayer_Base::Interact_Triggered()
{
	// This event is triggered by Enhanced Input when Hold Trigger is configured in Blueprint
	// We're not using this anymore - using manual time check in Interact_Completed instead
	// Keep this for compatibility if Hold Trigger is configured in Blueprint

	UE_LOG(LogTemp, Log, TEXT("Interact_Triggered - (Not used - using manual time check)"));
}

void APlayer_Base::OnInteractHoldCompleted()
{
	// Timer completed - this is a HOLD
	bInteractHoldTriggered = true;

	UE_LOG(LogTemp, Log, TEXT("OnInteractHoldCompleted - HOLD timer finished, executing swap"));

	// Check if there's an active interaction
	if (!Interactor)
		return;

	if (!Interactor->HasActiveInteraction())
		return;

	AActor* InteractionActor = Interactor->GetCurrentInteractionActor();
	if (!InteractionActor || !InteractionActor->Implements<UInteractable>())
		return;

	AController* PC = GetController();
	if (!PC)
		return;

	FInteractionContext Context;
	Context.InstigatorRef = PC;
	Context.InstigatorPawn = this;

	// HOLD - Execute interaction (swap weapon)
	FInteractionResult Result = IInteractable::Execute_ExecuteInteraction(InteractionActor, Context);

	if (Result.bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Hold interaction succeeded"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Hold interaction failed: %s"), *Result.FailureReason.ToString());
	}

	Interactor->StopCurrentInteraction();
}

void APlayer_Base::Interact_Completed()
{
	// Button released - cancel timer if still running
	if (GetWorld()->GetTimerManager().IsTimerActive(InteractHoldTimerHandle))
	{
		// Timer still active = button released before hold completed = SHORT PRESS
		GetWorld()->GetTimerManager().ClearTimer(InteractHoldTimerHandle);

		UE_LOG(LogTemp, Log, TEXT("Interact_Completed - SHORT PRESS detected, adding to inventory"));

		// Check if there's an active interaction
		if (!Interactor)
			return;

		if (!Interactor->HasActiveInteraction())
			return;

		AActor* InteractionActor = Interactor->GetCurrentInteractionActor();
		if (!InteractionActor || !InteractionActor->Implements<UInteractable>())
			return;

		AController* PC = GetController();
		if (!PC)
			return;

		FInteractionContext Context;
		Context.InstigatorRef = PC;
		Context.InstigatorPawn = this;

		// SHORT PRESS - Add to inventory or equip
		IInteractable::Execute_OnInteractionCancelled(InteractionActor, Context);

		Interactor->StopCurrentInteraction();
	}
	else
	{
		// Timer already finished = hold was completed
		UE_LOG(LogTemp, Log, TEXT("Interact_Completed - After HOLD (already handled)"));
	}
}

void APlayer_Base::Interact()
{
	// Legacy - direct call support
	Interact_Started();
	Interact_Triggered();
	Interact_Completed();
}
