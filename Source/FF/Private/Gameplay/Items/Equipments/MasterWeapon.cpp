// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Items/Equipments/MasterWeapon.h"
#include "Gameplay/Items/Equipments/GunAttackSystem.h"
#include "Gameplay/Characters/FFCharacter.h"
#include "Gameplay/Characters/Player_Base.h"
#include "Gameplay/Items/EquipmentSystem.h"
//#include "Gameplay/Items/Equipments/WeaponFireCameraShake.h"
#include "Gameplay/Data/WeaponData.h"
#include "Gameplay/Data/InteractionData.h"
#include "Gameplay/Characters/FFCharacter.h"
#include "Gameplay/Items/InventorySystem.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Gameplay/Components/HealthSystem.h"
#include "Gameplay/Components/Hurtbox.h"
#include "Kismet/KismetMathLibrary.h"
#include "Presentations/HUD/W_DynamicWeaponHUD.h"
#include "Presentations/HUD/W_MasterHUD.h"
#include "Gameplay/Interfaces/Damageable.h"
#include "Engine/DamageEvents.h"

// Sets default values
AMasterWeapon::AMasterWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMasterWeapon::BeginPlay()
{
    Super::BeginPlay();

    if (WeaponData && WeaponData->AttackComponentClass)
    {
        AttackSystem = NewObject<UWeaponAttackSystem>(
            this,
            WeaponData->AttackComponentClass,
            TEXT("Attack System")
        );
        AttackSystem->RegisterComponent();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("MasterWeapon::BeginPlay - WeaponData or AttackComponentClass is NULL!"));                                                                                                                    
        return;  
    }
    
    // Default state: Physics OFF, InteractCollision OFF
    // Physics and interaction are only enabled via SpawnDroppedWeapon or EnableWorldInteraction
    if (EquipmentMesh)
    {
        EquipmentMesh->SetSimulatePhysics(false);
        EquipmentMesh->SetEnableGravity(false);
    }
    if (InteractCollision)
    {
        InteractCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Try to get character reference if attached
    AFFCharacter* OwnerRef = Cast<AFFCharacter>(GetAttachParentActor());
    if (OwnerRef && AttackSystem)
    {
        AttackSystem->CharacterRef = OwnerRef;
    }
}

void AMasterWeapon::SetOwningCharacter(AFFCharacter* ToSetCharacter)
{
    if (AttackSystem)
    {
        AttackSystem->CharacterRef = ToSetCharacter;
    }
}

float AMasterWeapon::GetMaxAmmo() const
{
    return AttackSystem ? AttackSystem->GetMaxAmmo() : 0;
}

float AMasterWeapon::GetCurrentAmmo() const
{
    return AttackSystem ? AttackSystem->GetCurrentAmmo() : 0;
}

AMasterWeapon* AMasterWeapon::SpawnDroppedWeapon(UWorld* World, UWeaponData* WeaponData, const FVector& Location, const FRotator& Rotation, AActor* Owner)
{
    if (!World || !WeaponData || !WeaponData->EquipmentClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnDroppedWeapon: Invalid parameters"));
        return nullptr;
    }

    // Setup spawn parameters
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Owner;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // Spawn the weapon actor
    AActor* SpawnedActor = World->SpawnActor<AActor>(
        WeaponData->EquipmentClass,
        Location,
        Rotation,
        SpawnParams
    );

    AMasterWeapon* SpawnedWeapon = Cast<AMasterWeapon>(SpawnedActor);
    if (!SpawnedWeapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnDroppedWeapon: Failed to cast spawned actor to AMasterWeapon"));
        return nullptr;
    }

    // Set weapon data
    SpawnedWeapon->WeaponData = WeaponData;

    // Enable physics simulation for dropped weapon
    if (SpawnedWeapon->EquipmentMesh)
    {
        SpawnedWeapon->EquipmentMesh->SetSimulatePhysics(true);
        SpawnedWeapon->EquipmentMesh->SetEnableGravity(true);
        SpawnedWeapon->EquipmentMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        SpawnedWeapon->EquipmentMesh->SetCollisionResponseToAllChannels(ECR_Block);
    }

    // Enable interaction collision for dropped weapon
    if (SpawnedWeapon->InteractCollision)
    {
        SpawnedWeapon->InteractCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    }

    UE_LOG(LogTemp, Log, TEXT("SpawnDroppedWeapon: Spawned '%s' at %s with physics and interaction enabled"),
        *WeaponData->ItemName.ToString(),
        *Location.ToString());

    return SpawnedWeapon;
}

void AMasterWeapon::Attack()
{
    if (AttackSystem)
    {
        AttackSystem->PerformAttack();
    }
}

void AMasterWeapon::Reload()
{
    if (!AttackSystem || !WeaponData)
        return;

    // AttackSystem에게 재장전 가능 여부 확인
    if (!AttackSystem->CanReload())
        return;

    AttackSystem->bReloading = true;

    // 캐릭터 애니메이션 재생 (MasterWeapon이 조율)
    float ReloadDelay = 0.0f;
    if (AttackSystem->CharacterRef && WeaponData->BodyReloadMontage)
    {
        UAnimInstance* AnimInst = AttackSystem->CharacterRef->GetMesh()->GetAnimInstance();
        if (AnimInst)
        {
            ReloadDelay = AnimInst->Montage_Play(WeaponData->BodyReloadMontage, 1.f);
        }
    }

    // 무기 애니메이션 재생
    if (EquipmentMesh && WeaponData->WeaponReloadMontage)
    {
        EquipmentMesh->PlayAnimation(WeaponData->WeaponReloadMontage, false);
    }

    // 탄약 계산은 AttackSystem에 위임
    AttackSystem->ExecuteReload();

    // 타이머로 재장전 완료 처리
    FTimerHandle ReloadTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        ReloadTimerHandle,
        [this]()
        {
            // UI 업데이트 (MasterWeapon이 조율)
            if (AttackSystem && AttackSystem->CharacterRef)
            {
                APlayer_Base* Player = Cast<APlayer_Base>(AttackSystem->CharacterRef);
                if (Player && Player->MasterHUD)
                {
                    Player->MasterHUD->UpdateAmmoCount(
                        AttackSystem->GetMaxAmmo(),
                        AttackSystem->GetCurrentAmmo()
                    );
                }
            }
            if (AttackSystem)
            {
                AttackSystem->bReloading = false;
            }
        },
        ReloadDelay,
        false
    );
}

//==============================================================================
// IInteractable Interface Implementation
//==============================================================================

FInteractionResult AMasterWeapon::ExecuteInteraction_Implementation(const FInteractionContext& Context)
{
    if (!Context.InstigatorRef || !Context.InstigatorPawn)
    {
        return FInteractionResult::Failure(FText::FromString("Invalid instigator"));
    }

    // Get the player character
    APlayer_Base* Player = Cast<APlayer_Base>(Context.InstigatorPawn);
    if (!Player)
    {
        return FInteractionResult::Failure(FText::FromString("Only players can pick up weapons"));
    }

    // Get the equipment system
    UEquipmentSystem* EquipmentSys = Player->FindComponentByClass<UEquipmentSystem>();
    if (!EquipmentSys)
    {
        return FInteractionResult::Failure(FText::FromString("Player has no equipment system"));
    }

    // Get inventory system
    UInventorySystem* InventorySys = Player->FindComponentByClass<UInventorySystem>();

    // Check if we have weapon data
    if (!WeaponData)
    {
        return FInteractionResult::Failure(FText::FromString("Weapon has no data"));
    }

    // Determine which slot to equip to based on weapon type
    EEquipmentSlot TargetSlot = WeaponData->ValidSlot;

    // Check if the target slot already has a weapon equipped
    bool bSlotOccupied = EquipmentSys->IsEquipped(TargetSlot);

    if (bSlotOccupied)
    {
        // Slot is occupied - Hold completed, swap weapons
        UE_LOG(LogTemp, Log, TEXT("AMasterWeapon::ExecuteInteraction - Swapping weapons"));

        // Unequip current weapon (this will drop it)
        bool bShouldEquipToHand = EquipmentSys->CurrentEquippedSlot == TargetSlot;
        UWeaponData* DroppedWeaponData = EquipmentSys->Unequip(TargetSlot);

        // Spawn dropped weapon actor in world
        if (DroppedWeaponData)
        {
            // Calculate spawn location in front of player
            FVector PlayerLocation = Player->GetActorLocation();
            FVector PlayerForward = Player->GetActorForwardVector();
            FVector SpawnLocation = PlayerLocation + (PlayerForward * 100.0f) + FVector(0.0f, 0.0f, -50.0f);
            FRotator SpawnRotation = Player->GetActorRotation();

            // Spawn dropped weapon with physics enabled
            AMasterWeapon* DroppedWeapon = SpawnDroppedWeapon(
                GetWorld(),
                DroppedWeaponData,
                SpawnLocation,
                SpawnRotation,
                Player
            );

            if (!DroppedWeapon)
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to spawn dropped weapon"));
            }
        }
        
        // Equip new weapon
        EquipmentSys->Equip(TargetSlot, WeaponData, bShouldEquipToHand);

        // Destroy the world weapon actor
        Destroy();

        return FInteractionResult::Success();
    }
    else
    {
        // Slot is empty - equip directly
        EquipmentSys->Equip(TargetSlot, WeaponData);

        // Destroy the world weapon actor (since it's now equipped)
        Destroy();

        UE_LOG(LogTemp, Log, TEXT("AMasterWeapon::ExecuteInteraction - Weapon equipped to slot"));
        return FInteractionResult::Success();
    }
}

bool AMasterWeapon::CanInteract_Implementation(AController* InstigatorRef) const
{
    if (!InstigatorRef)
        return false;

    // Check if the weapon is already equipped
    if (GetAttachParentActor())
    {
        // Weapon is already attached to someone
        return false;
    }

    return true;
}

FText AMasterWeapon::GetInteractionPrompt_Implementation() const
{
    // Try to get player and equipment system to check slot status
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        APlayer_Base* Player = Cast<APlayer_Base>(PC->GetPawn());
        if (Player)
        {
            UEquipmentSystem* EquipmentSys = Player->FindComponentByClass<UEquipmentSystem>();
            if (EquipmentSys && WeaponData)
            {
                EEquipmentSlot TargetSlot = WeaponData->ValidSlot;
                bool bSlotOccupied = EquipmentSys->IsEquipped(TargetSlot);

                FString WeaponName = WeaponData->ItemName.IsEmpty()
                    ? TEXT("weapon")
                    : WeaponData->ItemName.ToString();

                if (bSlotOccupied)
                {
                    // Slot occupied - Hold to swap, short press to add to inventory
                    return FText::Format(
                        FText::FromString("[E] Add {0} to inventory | [Hold E] Swap weapons"),
                        FText::FromString(WeaponName)
                    );
                }
                else
                {
                    // Slot empty - will equip directly
                    return FText::Format(
                        FText::FromString("[E] Pick up {0}"),
                        FText::FromString(WeaponName)
                    );
                }
            }
        }
    }

    // Fallback
    if (WeaponData && !WeaponData->ItemName.IsEmpty())
    {
        return FText::Format(
            FText::FromString("[E] Pick up {0}"),
            WeaponData->ItemName
        );
    }

    return FText::FromString("[E] Pick up weapon");
}

bool AMasterWeapon::IsHoldInteraction_Implementation() const
{
    // Weapons always use hold interaction system
    // - Short press (release before hold): add to inventory or equip if slot empty
    // - Hold complete: swap weapons if slot occupied
    return true;
}

float AMasterWeapon::GetHoldDuration_Implementation() const
{
    // Hold duration for weapon swap (1 second)
    return 1.0f;
}

bool AMasterWeapon::IsSingleUse_Implementation() const
{
    // Weapon can only be picked up once (until dropped again)
    return true;
}

void AMasterWeapon::SetHighlighted_Implementation(bool bHighlight)
{
    bIsHighlighted = bHighlight;

    // Apply highlight effect to weapon mesh
    if (EquipmentMesh)
    {
        if (bHighlight)
        {
            // Enable custom depth for outline effect
            EquipmentMesh->SetRenderCustomDepth(true);
            EquipmentMesh->SetCustomDepthStencilValue(1);
        }
        else
        {
            // Disable custom depth
            EquipmentMesh->SetRenderCustomDepth(false);
        }
    }
}

void AMasterWeapon::OnInteractionStarted_Implementation(const FInteractionContext& Context)
{
    // Not used - Hold is managed by Enhanced Input
}

void AMasterWeapon::OnInteractionCancelled_Implementation(const FInteractionContext& Context)
{
    // Short press - add to inventory
    if (!Context.InstigatorPawn)
        return;

    APlayer_Base* Player = Cast<APlayer_Base>(Context.InstigatorPawn);
    if (!Player)
        return;

    UEquipmentSystem* EquipmentSys = Player->FindComponentByClass<UEquipmentSystem>();
    UInventorySystem* InventorySys = Player->FindComponentByClass<UInventorySystem>();

    if (!WeaponData)
        return;

    EEquipmentSlot TargetSlot = WeaponData->ValidSlot;
    bool bSlotOccupied = EquipmentSys && EquipmentSys->IsEquipped(TargetSlot);

    if (bSlotOccupied)
    {
        // Slot occupied - add to inventory
        if (InventorySys)
        {
            // Try to add weapon to inventory
            bool bAdded = InventorySys->TryAddItemEmptySpot(WeaponData, 1);

            if (bAdded)
            {
                UE_LOG(LogTemp, Log, TEXT("AMasterWeapon::OnInteractionCancelled - Added %s to inventory"),
                    *WeaponData->ItemName.ToString());

                // Destroy the world weapon actor
                Destroy();
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("AMasterWeapon::OnInteractionCancelled - Inventory is full"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No inventory system available"));
        }
    }
    else
    {
        // Slot empty - equip directly (short press on empty slot)
        if (EquipmentSys)
        {
            EquipmentSys->Equip(TargetSlot, WeaponData);
            Destroy();
            UE_LOG(LogTemp, Log, TEXT("AMasterWeapon::OnInteractionCancelled - Weapon equipped"));
        }
    }
}

AActor* AMasterWeapon::GetInteractableActor_Implementation()
{
    return this;
}

EInteractiveType AMasterWeapon::GetInteractionType_Implementation() const
{
    return EInteractiveType::WeaponPickup;
}
