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
#include "Presentations/HUD/W_MasterHUD.h"
#include "GameplayTagContainer.h"
#include "Gameplay/Data/ConsumableData.h"
#include "Gameplay/Components/HealthSystem.h"

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
		// 공격 입력 라우팅
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &APlayer_Base::OnAttackStarted);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &APlayer_Base::OnAttackHeld);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &APlayer_Base::OnAttackReleased);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Canceled, this, &APlayer_Base::OnAttackReleased);

		EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Triggered, this, &APlayer_Base::Inventory);
		EnhancedInputComponent->BindAction(FlashOnAction, ETriggerEvent::Triggered, this, &AFFCharacter::FlashOnOff);

		// Interact actions - Hold support
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APlayer_Base::Interact_Started);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &APlayer_Base::Interact_Completed);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Canceled, this, &APlayer_Base::Interact_Completed);
	}
}

void APlayer_Base::BeginPlay()
{
	Super::BeginPlay();

	FlashlightChild->SetVisibility(false);

	if (InventorySystem)
	{
		InventorySystem->OnConsumableUsed.AddDynamic(this, &APlayer_Base::HandleConsumableUsed);
	}
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

void APlayer_Base::OpenLootingUI(UInventorySystem* ContainerInven)
{
	PendingLootContainer = ContainerInven;
	OpenUIWithContext(EInventoryUIContext::Looting);
	// LootingInteract는 OpenMontage 종료 후 OnOpenMontageEnded에서 호출됨
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
	else if (EquipmentSystem->CurrentEquippedSlot == EEquipmentSlot::Secondary)
	{
		Weapon = Cast<AMasterWeapon>(SecondaryChild->GetChildActor());
	}
	else if (EquipmentSystem->CurrentEquippedSlot == EEquipmentSlot::Handgun)
	{
		Weapon = Cast<AMasterWeapon>(HandgunChild->GetChildActor());
	}
	else if (EquipmentSystem->CurrentEquippedSlot == EEquipmentSlot::Melee)
	{
		Weapon = Cast<AMasterWeapon>(MeleeChildActor->GetChildActor());
	}
	else
	{
		PlayerHUD->HideWeaponUI();
		return;
	}

	if (!Weapon || !Weapon->AttackSystem)
	{
		PlayerHUD->HideWeaponUI();
		return;
	}

	MasterHUD = PlayerHUD->ShowWeaponUI(
		WeaponData,
		Weapon->GetMaxAmmo(),
		Weapon->GetCurrentAmmo());

	if (MasterHUD)
	{
		MasterHUD->CharacterRef = this;
	}

	PlayerHUD->SetWeaponDataOnHUD(
		WeaponData->WeaponUITexture,
		WeaponData->ItemName.ToString(),
		CurrentWeapon->GetMaxAmmo(),
		CurrentWeapon->GetCurrentAmmo()
	);
}

void APlayer_Base::OnAttackStarted(const FInputActionValue& Value)
{
	if (!CanAttack())
		return;

	if (CurrentWeapon)
	{
		CurrentWeapon->OnAttackStarted();
	}
}

void APlayer_Base::OnAttackHeld(const FInputActionValue& Value)
{
	if (!CanAttack())
		return;

	if (CurrentWeapon)
	{
		CurrentWeapon->OnAttackHeld();
	}
}

void APlayer_Base::OnAttackReleased(const FInputActionValue& Value)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->OnAttackReleased();
	}
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
	else if (EquipmentSystem->CurrentEquippedSlot == EEquipmentSlot::Secondary)
	{
		MasterWeapon = Cast<AMasterWeapon>(SecondaryChild->GetChildActor());
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

	// Check if there's an active interaction
	if (!Interactor || !Interactor->HasActiveInteraction())
		return;

	AActor* InteractionActor = Interactor->GetCurrentInteractionActor();
	if (!InteractionActor || !InteractionActor->Implements<UInteractable>())
		return;
	// TODO: 상호작용 타입에 따라 무조건 홀드인지 체크
	// Check if this is a hold interaction
	bool bIsHoldInteraction = IInteractable::Execute_IsHoldInteraction(InteractionActor);
	bool bNeedsHoldUI = bIsHoldInteraction;
	if (IInteractable::Execute_GetInteractionType(InteractionActor) == EInteractiveType::Container)
	{
		// Get hold duration from interactable
		CurrentInteractionHoldDuration = IInteractable::Execute_GetHoldDuration(InteractionActor);
		if (CurrentInteractionHoldDuration <= 0.0f)
		{
			CurrentInteractionHoldDuration = HoldThreshold;
		}

		// Record the start time for hold detection
		InteractStartTime = GetWorld()->GetTimeSeconds();

		// Start timer for hold detection
		GetWorld()->GetTimerManager().SetTimer(
			InteractHoldTimerHandle,
			this,
			&APlayer_Base::OnInteractHoldCompleted,
			CurrentInteractionHoldDuration,
			false
		);

		// Start interaction UI
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC)
		{
			APlayerHUD* PlayerHUD = Cast<APlayerHUD>(PC->GetHUD());
			if (PlayerHUD)
			{
				FText PromptText = IInteractable::Execute_GetInteractionPrompt(InteractionActor);
				PlayerHUD->StartInteractionUI(PromptText);

				// Start UI update timer (60 FPS)
				GetWorld()->GetTimerManager().SetTimer(
					InteractionUIUpdateHandle,
					this,
					&APlayer_Base::UpdateInteractionUIProgress,
					1.0f / 60.0f,
					true
				);
			}
		}

		UE_LOG(LogTemp, Log, TEXT("Interact_Started - HOLD interaction started for %.2f seconds"), CurrentInteractionHoldDuration);
	}
	else if (IInteractable::Execute_GetInteractionType(InteractionActor) == EInteractiveType::Pickup)
	{
		// Slot empty or non-hold interaction - execute immediately
		AController* Ctrl = GetController();
		if (!Ctrl)
			return;

		FInteractionContext Context;
		Context.InstigatorRef = Ctrl;
		Context.InstigatorPawn = this;

		FInteractionResult Result = IInteractable::Execute_ExecuteInteraction(InteractionActor, Context);

		if (Result.bSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("Immediate interaction succeeded (slot empty or non-hold)"));
		}

		Interactor->StopCurrentInteraction();
	}
	else if (IInteractable::Execute_GetInteractionType(InteractionActor) == EInteractiveType::WeaponPickup)
	{
		// For weapons, check if the target slot is occupied
		// If slot is empty, execute immediately without hold/UI
		AMasterWeapon* WeaponActor = Cast<AMasterWeapon>(InteractionActor);
		if (WeaponActor && WeaponActor->WeaponData && EquipmentSystem)
		{
			EEquipmentSlot TargetSlot = WeaponActor->WeaponData->ValidSlot;
			bool bSlotOccupied = EquipmentSystem->IsEquipped(TargetSlot);

			if (!bSlotOccupied)
			{
				// Slot is empty - equip directly without hold/UI
				bNeedsHoldUI = false;
			}
		}

		if (bNeedsHoldUI)
		{
			// Get hold duration from interactable
			CurrentInteractionHoldDuration = IInteractable::Execute_GetHoldDuration(InteractionActor);
			if (CurrentInteractionHoldDuration <= 0.0f)
			{
				CurrentInteractionHoldDuration = HoldThreshold;
			}

			// Record the start time for hold detection
			InteractStartTime = GetWorld()->GetTimeSeconds();

			// Start timer for hold detection
			GetWorld()->GetTimerManager().SetTimer(
				InteractHoldTimerHandle,
				this,
				&APlayer_Base::OnInteractHoldCompleted,
				CurrentInteractionHoldDuration,
				false
			);

			// Start interaction UI
			APlayerController* PC = Cast<APlayerController>(GetController());
			if (PC)
			{
				APlayerHUD* PlayerHUD = Cast<APlayerHUD>(PC->GetHUD());
				if (PlayerHUD)
				{
					FText PromptText = IInteractable::Execute_GetInteractionPrompt(InteractionActor);
					PlayerHUD->StartInteractionUI(PromptText);

					// Start UI update timer (60 FPS)
					GetWorld()->GetTimerManager().SetTimer(
						InteractionUIUpdateHandle,
						this,
						&APlayer_Base::UpdateInteractionUIProgress,
						1.0f / 60.0f,
						true
					);
				}
			}

			UE_LOG(LogTemp, Log, TEXT("Interact_Started - HOLD interaction started for %.2f seconds"), CurrentInteractionHoldDuration);
		}
		else
		{
			// Slot empty or non-hold interaction - execute immediately
			AController* Ctrl = GetController();
			if (!Ctrl)
				return;

			FInteractionContext Context;
			Context.InstigatorRef = Ctrl;
			Context.InstigatorPawn = this;

			FInteractionResult Result = IInteractable::Execute_ExecuteInteraction(InteractionActor, Context);

			if (Result.bSuccess)
			{
				UE_LOG(LogTemp, Log, TEXT("Immediate interaction succeeded (slot empty or non-hold)"));
			}

			Interactor->StopCurrentInteraction();
		}
	}
	
}

void APlayer_Base::Interact_Completed()
{
	// Button released - cancel timer if still running
	if (GetWorld()->GetTimerManager().IsTimerActive(InteractHoldTimerHandle))
	{
		// Timer still active = button released before hold completed = CANCELLED
		GetWorld()->GetTimerManager().ClearTimer(InteractHoldTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(InteractionUIUpdateHandle);

		// Cancel interaction UI
		APlayerController* PlayerPC = Cast<APlayerController>(GetController());
		if (PlayerPC)
		{
			APlayerHUD* PlayerHUD = Cast<APlayerHUD>(PlayerPC->GetHUD());
			if (PlayerHUD)
			{
				PlayerHUD->CancelInteractionUI();
			}
		}

		UE_LOG(LogTemp, Log, TEXT("Interact_Completed - HOLD cancelled, interaction cancelled"));

		// Check if there's an active interaction
		if (!Interactor)
			return;

		if (!Interactor->HasActiveInteraction())
			return;

		AActor* InteractionActor = Interactor->GetCurrentInteractionActor();
		if (!InteractionActor || !InteractionActor->Implements<UInteractable>())
			return;

		AController* Ctrl = GetController();
		if (!Ctrl)
			return;

		FInteractionContext Context;
		Context.InstigatorRef = Ctrl;
		Context.InstigatorPawn = this;

		// CANCELLED - notify interactable
		IInteractable::Execute_OnInteractionCancelled(InteractionActor, Context);

		Interactor->StopCurrentInteraction();
	}
	else
	{
		// Timer already finished = hold was completed
		UE_LOG(LogTemp, Log, TEXT("Interact_Completed - After HOLD (already handled)"));
	}
}

void APlayer_Base::UpdateInteractionUIProgress()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float ElapsedTime = CurrentTime - InteractStartTime;
	float Progress = FMath::Clamp(ElapsedTime / CurrentInteractionHoldDuration, 0.0f, 1.0f);

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		APlayerHUD* PlayerHUD = Cast<APlayerHUD>(PC->GetHUD());
		if (PlayerHUD)
		{
			PlayerHUD->UpdateInteractionUI(Progress);
		}
	}
}

void APlayer_Base::OnInteractHoldCompleted()
{
	// Timer completed - this is a HOLD
	bInteractHoldTriggered = true;

	// Stop UI update timer
	GetWorld()->GetTimerManager().ClearTimer(InteractionUIUpdateHandle);

	// Complete interaction UI
	APlayerController* PlayerPC = Cast<APlayerController>(GetController());
	if (PlayerPC)
	{
		APlayerHUD* PlayerHUD = Cast<APlayerHUD>(PlayerPC->GetHUD());
		if (PlayerHUD)
		{
			PlayerHUD->CompleteInteractionUI();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("OnInteractHoldCompleted - HOLD timer finished, executing swap"));

	// Check if there's an active interaction
	if (!Interactor)
		return;

	if (!Interactor->HasActiveInteraction())
		return;

	AActor* InteractionActor = Interactor->GetCurrentInteractionActor();
	if (!InteractionActor || !InteractionActor->Implements<UInteractable>())
		return;
	
	AController* Ctrl = GetController();
	if (!Ctrl)
		return;

	FInteractionContext Context;
	Context.InstigatorRef = Ctrl;
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

void APlayer_Base::Interact()
{
	// Legacy - direct call support
	Interact_Started();
	Interact_Completed();
}

void APlayer_Base::OpenUIWithContext(EInventoryUIContext Context)
{
	APC_Base* PC = Cast<APC_Base>(GetController());
	if (!PC)
		return;

	const FInventoryAnimSet* AnimSet = InventoryAnimSets.Find(Context);
	if (!AnimSet || !AnimSet->OpenMontage || !AnimSet->CloseMontage)
		return;

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!AnimInst)
		return;

	CurrentUIContext = Context;

	AnimInst->Montage_Play(AnimSet->OpenMontage, 0.75f);

	FGameplayTag InventoryModeTag = FGameplayTag::RequestGameplayTag(FName("EnhancedInput.Modes.Inventory"));
	PC->AddInputModeTag(InventoryModeTag);
	PC->SetIgnoreInput(true);

	FOnMontageEnded OpenDelegate;
	OpenDelegate.BindUObject(this, &APlayer_Base::OnOpenMontageEnded);
	AnimInst->Montage_SetEndDelegate(OpenDelegate, AnimSet->OpenMontage);
}

void APlayer_Base::CloseInventory(UAnimInstance* AnimInst)
{
	const FInventoryAnimSet* AnimSet = InventoryAnimSets.Find(CurrentUIContext);
	if (!AnimSet || !AnimSet->CloseMontage)
		return;

	AnimInst->Montage_Play(AnimSet->CloseMontage, 0.75f);

	FOnMontageEnded CloseDelegate;
	CloseDelegate.BindUObject(this, &APlayer_Base::OnCloseMontageEnded);
	AnimInst->Montage_SetEndDelegate(CloseDelegate, AnimSet->CloseMontage);
}

void APlayer_Base::Inventory()
{
	APC_Base* PC = Cast<APC_Base>(GetController());
	if (!PC)
		return;

	if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
	{
		if (PC->IsVisibleWidget())
		{
			CloseInventory(AnimInst);
		}
		else
		{
			OpenUIWithContext(EInventoryUIContext::Inventory);
		}
	}
}

void APlayer_Base::OnOpenMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
	{
		AnimInst->Montage_JumpToSection("Idle", Montage);
	}

	if (CurrentUIContext == EInventoryUIContext::Looting)
	{
		if (APC_Base* PC = Cast<APC_Base>(GetController()))
		{
			PC->LootingInteract(PendingLootContainer);
		}
		PendingLootContainer = nullptr;
	}
}

void APlayer_Base::OnCloseMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	APC_Base* PC = Cast<APC_Base>(GetController());
	if (!PC)
		return;

	FGameplayTag InventoryModeTag = FGameplayTag::RequestGameplayTag(FName("EnhancedInput.Modes.Inventory"));
	PC->RemoveInputModeTag(InventoryModeTag);
	PC->SetIgnoreInput(false);

	CurrentUIContext = EInventoryUIContext::None;
}

void APlayer_Base::HandleConsumableUsed(UConsumableData* ConsumableData, const FItemSlot& Slot)
{
	switch (ConsumableData->EffectType)
	{
	case EConsumableEffect::Hunger:
		break;
	case EConsumableEffect::Heal:
		HealthComponent->Heal(ConsumableData->EffectValue);
		break;
	case EConsumableEffect::Thirst:
		break;
	}
}
