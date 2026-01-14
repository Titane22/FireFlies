// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Items/Equipments/MasterWeapon.h"
#include "Gameplay/Items/Equipments/WeaponSystem.h"
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
#include "Gameplay/Interfaces/Damageable.h"
#include "Engine/DamageEvents.h"

// Sets default values
AMasterWeapon::AMasterWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // Create Components
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    InteractCollision = CreateDefaultSubobject<USphereComponent>(TEXT("InteractCollision"));
    WeaponSystem = CreateDefaultSubobject<UWeaponSystem>(TEXT("WeaponSystem"));

    // Setting up the component hierarchy
    RootComponent = WeaponMesh;
    InteractCollision->SetupAttachment(WeaponMesh);

    // Set InteractBox collision preset to Interactable
    InteractCollision->SetCollisionProfileName(FName("Interactable"));
    bReloading = false;
    bAutoReload = false;

    WeaponSystem->bIsDryAmmo = false;
    // Weapon_Details는 BeginPlay에서 WeaponData로부터 초기화됩니다
}

// Called when the game starts or when spawned
void AMasterWeapon::BeginPlay()
{
    Super::BeginPlay();

    AFFCharacter* OwnerRef = Cast<AFFCharacter>(GetAttachParentActor());
    if (!OwnerRef)
        return;
    WeaponSystem->CharacterRef = OwnerRef;

    // WeaponData로부터 탄약 정보 로드
    if (WeaponData)
    {
        WeaponSystem->Weapon_Details.Weapon_Data.CurrentAmmo = WeaponData->CurrentAmmo;
        WeaponSystem->Weapon_Details.Weapon_Data.MaxAmmo = WeaponData->MaxAmmo;
        WeaponSystem->Weapon_Details.Weapon_Data.ClipAmmo = WeaponData->ClipAmmo;
        WeaponSystem->Weapon_Details.Weapon_Data.DifferentAmmo = WeaponData->DifferentAmmo;
        WeaponSystem->Weapon_Details.Weapon_Data.Ammo_Count = WeaponData->AmmoCount;
        WeaponSystem->Weapon_Details.Weapon_Data.ShortGun_Trace = WeaponData->bShortGunTrace;

        UE_LOG(LogTemp, Log, TEXT("MasterWeapon::BeginPlay - Loaded ammo from WeaponData: CurrentAmmo=%d, MaxAmmo=%d"),
            WeaponData->CurrentAmmo, WeaponData->MaxAmmo);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MasterWeapon::BeginPlay - WeaponData is not assigned! Using default ammo values."));
    }

    // TODO: 월드 스폰시에는 물리 적용 안되나, 드롭시에는 적용
    WeaponMesh->SetSimulatePhysics(true);
    WeaponMesh->SetEnableGravity(true);
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

void AMasterWeapon::FireBullet(FHitResult Hit, bool bReturnHit)
{
    for (int32 curBurst = 0; curBurst < WeaponData->BurstAmount; curBurst++)
    {
        float PointX, PointY;
        RandPointInCircle(FMath::Tan(WeaponData->BulletSpread) * 10.0f, PointX, PointY);

        // Get the player camera manager for the bullet direction
        APlayerCameraManager* CameraManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
        if (!CameraManager)
        {
            return;
        }
        FVector SpreadAdjustedHitLocation = Hit.Location + CameraManager->GetActorRightVector() * PointX + CameraManager->GetActorUpVector() * PointY;
        FVector MuzzleLocation = WeaponMesh->GetSocketLocation(FName("Muzzle"));

        // BulletDirection represents the direction from the muzzle to the target.
        // Calculate the direction vector of the trajectory 
        // by subtracting the aim point position from the muzzle position.
        FVector BulletDirection = MuzzleLocation - SpreadAdjustedHitLocation;
        
        // Setup trace parameters
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        
        // Create array of actors to ignore
        if (!WeaponSystem)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("WeaponSystem is NULL"));
            return;
        }
        else if (!WeaponSystem->CharacterRef)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("WeaponSystem->CharacterRef is NULL"));
            return;
        }
        QueryParams.AddIgnoredActor(Cast<AActor>(WeaponSystem->CharacterRef));

        // ===== 디버그 라인 활성화 =====
        // DrawDebugLine(
        //     GetWorld(),           // 월드
        //     MuzzleLocation,        // 시작점
        //     MuzzleLocation + (BulletDirection * -5.0f),          // 끝점
        //     FColor::Yellow,       // 라인 색상
        //     false,               // 지속적으로 그릴지 여부
        //     5.0f,                // 지속 시간 (초)
        //     0,                   // 우선순위
        //     2.0f                 // 두께
        // );
        // ==============================
        // Perform line trace
        FHitResult HitResult;
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            MuzzleLocation,
            MuzzleLocation + (BulletDirection * -5.0f),
            COLLISION_BULLET,
            QueryParams
        );
        if (!bHit)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Anyone not Hit!!!!!!"));
            return;
        }

        // Process hit result
        if (HitResult.GetActor())
        {
            // Check if hit component is simulating physics
            UPrimitiveComponent* HitComponent = HitResult.GetComponent();
            if (HitComponent && HitComponent->IsSimulatingPhysics())
            {
                // Apply physics impulse at impact point
                FVector ImpulseDir = -BulletDirection.GetSafeNormal();
                HitComponent->AddImpulseAtLocation(ImpulseDir * -1000.0f, HitResult.Location);
            }

            // Apply damage to hit actor
            bool bValidHit;
            bool bKilledPlayer = ApplyHit(HitResult, bValidHit);
            
            if (bValidHit)
            {
                // HitMarker
                if (WeaponData && WeaponData->HitMarkerUI)
                {
                    // TODO: Hit Marker
                    // UUserWidget* UIHitMarker = CreateWidget<UUserWidget>(GetWorld()->GetFirstPlayerController(), WeaponData->HitMarkerUI);
                    // if (UIHitMarker)
                    // {
                    //     UIHitMarker->AddToViewport();
                    // }
                }
            }
            if (bKilledPlayer)
            {
                // PlaySound2D
                if (WeaponData && WeaponData->KillSound)
                {
                    UGameplayStatics::PlaySound2D(
                        this,
                        WeaponData->KillSound,
                        1.0f,
                        1.0f,
                        0.0f,
                        nullptr,
                        nullptr,
                        true
                    );
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("MasterWeapon::FireBullet::KillSound is NULL"));
                }
            }

            // Spawn bullet trace effect
            if (WeaponData && WeaponData->BulletTraceClass)
            {
                FVector BulletEndLocation = HitResult.bBlockingHit ? HitResult.ImpactPoint : HitResult.TraceEnd;
                BulletEndLocation = BulletEndLocation - MuzzleLocation;

                FTransform SpawnTransform;
                SpawnTransform.SetLocation(WeaponMesh->GetSocketLocation(FName("Muzzle")));
                SpawnTransform.SetRotation(BulletEndLocation.Rotation().Quaternion());
                SpawnTransform.SetScale3D(FVector(1.0f, 1.0f, 1.0f));

                FActorSpawnParameters SpawnParams;
                SpawnParams.Owner = this;
                SpawnParams.Instigator = GetInstigator();

                GetWorld()->SpawnActor<AActor>(WeaponData->BulletTraceClass, SpawnTransform, SpawnParams);
            }
        }

        Hit = HitResult;
        bReturnHit = true;
    }
}

void AMasterWeapon::FireFX()
{
    if (!WeaponData)
    {
        UE_LOG(LogTemp, Warning, TEXT("MasterWeapon::FireFX - WeaponData is NULL"));
        return;
    }

    if (!WeaponData->SoundAttenuation)
    {
        UE_LOG(LogTemp, Warning, TEXT("MasterWeapon::FireFX - SoundAttenuation is NULL"));
        return;
    }

    if (!WeaponData->SoundConcurrency)
    {
        UE_LOG(LogTemp, Warning, TEXT("MasterWeapon::FireFX - SoundConcurrency is NULL"));
        return;
    }

    WeaponSystem->FireFX(WeaponData->FireSound, WeaponMesh->GetSocketLocation("Muzzle"), WeaponData->SoundAttenuation, WeaponData->SoundConcurrency);

    if (!WeaponData->MuzzleFlashVFX)
    {
        UE_LOG(LogTemp, Warning, TEXT("MasterWeapon::FireFX - MuzzleFlashVFX is NULL"));
        return;
    }

    WeaponSystem->MuzzleVFX(WeaponData->MuzzleFlashVFX, Muzzle);

    WeaponSystem->FireMontage(WeaponData->BodyFireMontage);

    WeaponMesh->PlayAnimation(WeaponData->WeaponFireMontage, false);
}

void AMasterWeapon::FireBlankTracer()
{
    if (!WeaponData || !WeaponData->BulletTraceClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("MasterWeapon::FireBlankTracer - WeaponData or BulletTraceClass is NULL"));
        return;
    }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->PlayerCameraManager)
        return;

    // Apply camera shake (only for players)
    ApplyCameraShake(PC);

    FVector SocketLocation = WeaponMesh->GetSocketLocation(FName("Muzzle"));
    FVector TraceEndLocation = PC->PlayerCameraManager->GetRootComponent()->GetComponentLocation()
        + PC->PlayerCameraManager->GetActorForwardVector() * 20000.0f;
    FVector DirectionVector = TraceEndLocation - SocketLocation;
    FRotator Rotation = UKismetMathLibrary::MakeRotFromX(DirectionVector);

    FTransform NewTransform(Rotation, SocketLocation, FVector(1.0f));
    GetWorld()->SpawnActor<AActor>(WeaponData->BulletTraceClass, NewTransform);
}

void AMasterWeapon::RandPointInCircle(float Radius, float& PointX, float& PointY)
{
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float DistanceFromCenter = FMath::RandRange(0.0f, Radius);
    
    // Convert polar coordinates to cartesian coordinates
    PointX = DistanceFromCenter * FMath::Cos(FMath::DegreesToRadians(Angle));
    PointY = DistanceFromCenter * FMath::Sin(FMath::DegreesToRadians(Angle));
}

void AMasterWeapon::SetOwningCharacter(AFFCharacter* ToSetCharacter)
{
    if (WeaponSystem)
    {
        WeaponSystem->CharacterRef = ToSetCharacter;
    }
}

float AMasterWeapon::GetMaxAmmo() const
{
    return WeaponSystem->Weapon_Details.Weapon_Data.MaxAmmo;
}

float AMasterWeapon::GetCurrentAmmo() const
{
    return WeaponSystem->Weapon_Details.Weapon_Data.CurrentAmmo;
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
    if (SpawnedWeapon->WeaponMesh)
    {
        SpawnedWeapon->WeaponMesh->SetSimulatePhysics(true);
        SpawnedWeapon->WeaponMesh->SetEnableGravity(true);
        SpawnedWeapon->WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        SpawnedWeapon->WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);

        UE_LOG(LogTemp, Log, TEXT("SpawnDroppedWeapon: Spawned '%s' at %s with physics enabled"),
            *WeaponData->ItemName.ToString(),
            *Location.ToString());
    }

    return SpawnedWeapon;
}

bool AMasterWeapon::ApplyHit(const FHitResult HitResult, bool& ValidHit)
{
    AActor* HitActor = HitResult.GetActor();
    
    // Hit된 액터가 있고, 그 액터가 유효한지 확인
    if (!HitActor)
    {
        ValidHit = false;
        return false;
    }
    
    bool bIsDead = false;

    if (HitActor->Implements<UDamageable>())
    {
        FPointDamageEvent DamageEvent(
              WeaponData->Damage,           // Damage
              HitResult,                    // HitInfo
              -HitResult.ImpactNormal,      // ShotDirection (총알 방향)
              nullptr                       // DamageTypeClass
          );


        APawn* OwnerPawn = WeaponSystem->CharacterRef;
        AController* OwnerController = OwnerPawn ? OwnerPawn->GetController() : nullptr;

        float ActualDamage = IDamageable::Execute_TakeDamage(
            HitActor,
            WeaponData->Damage,
            DamageEvent,
            HitResult.BoneName,
            OwnerController,
            this
        );
        ValidHit = ActualDamage > 0.0f;
        if (ValidHit)
        {
            if (WeaponData && WeaponData->HitMarkerSound)
            {
                UGameplayStatics::PlaySound2D(
                    this,                       // WorldContextObject
                    WeaponData->HitMarkerSound, // Sound
                    1.0f,                       // Volume Multiplier
                    1.0f,                       // Pitch Multiplier
                    0.0f,                       // Start Time
                    nullptr,                    // Concurrency Settings
                    nullptr,                    // Owning Actor
                    true                        // Is UI Sound
                );
            }
        }
        bIsDead = IDamageable::Execute_IsDead(HitActor);
    }
    
    return bIsDead;
}

void AMasterWeapon::ApplyCameraShake(APlayerController* PC)
{
    // Only apply camera shake for player-controlled characters
    if (!PC || !WeaponSystem || !WeaponSystem->CharacterRef)
        return;

    APlayer_Base* Player = Cast<APlayer_Base>(WeaponSystem->CharacterRef);
    if (Player)
    {
        // TODO: PC->ClientStartCameraShake(UWeaponFireCameraShake::StaticClass(), 1.0f);
    }
}

bool AMasterWeapon::PerformCameraTrace(APlayerCameraManager* CameraManager, FHitResult& OutHitResult)
{
    if (!CameraManager || !WeaponData)
        return false;
    APlayer_Base* Player = Cast<APlayer_Base>(WeaponSystem->CharacterRef);
    if (!Player)
        return false;

    APlayerCameraManager* PCM = Player->GetPlayerCameraManager();
    if (!PCM)
        return false;
    
    FVector StartLocation = PCM->GetCameraLocation();
    FVector ForwardVector = PCM->GetActorForwardVector();
    FVector EndLocation = StartLocation + (ForwardVector * WeaponData->MaxRange);
    UE_LOG(LogTemp, Warning, TEXT("[PerformCameraTrace] StartLocation: %s, ForwardVector: %s, EndLocation: %s"),
                *StartLocation.ToString(), *ForwardVector.ToString(), *EndLocation.ToString());
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    // DrawDebugLine(
    //     GetWorld(),           // 월드
    //     StartLocation,        // 시작점
    //     EndLocation,          // 끝점
    //     FColor::Green,       // 라인 색상
    //     false,               // 지속적으로 그릴지 여부
    //     5.0f,                // 지속 시간 (초)
    //     0,                   // 우선순위
    //     2.0f                 // 두께
    // );
    return GetWorld()->LineTraceSingleByChannel(
        OutHitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
}

void AMasterWeapon::ExecuteFireSequence(const FHitResult& CameraHitResult)
{
    FVector MuzzleLocation = WeaponMesh->GetSocketLocation(FName("Muzzle"));
    FVector DirectionToTarget = MuzzleLocation - CameraHitResult.Location;
    GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow,
        FString::Printf(TEXT("Hit Actor: %s"), CameraHitResult.GetActor() ? *CameraHitResult.GetActor()->GetName() : TEXT("NULL")));
    // Perform muzzle trace
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    // DrawDebugLine(
    //     GetWorld(),           // 월드
    //     MuzzleLocation,        // 시작점
    //     MuzzleLocation + (DirectionToTarget * -500.0f),          // 끝점
    //     FColor::Yellow,       // 라인 색상
    //     false,               // 지속적으로 그릴지 여부
    //     5.0f,                // 지속 시간 (초)
    //     0,                   // 우선순위
    //     2.0f                 // 두께
    // );
    FHitResult MuzzleHitResult;
    bool bMuzzleHit = GetWorld()->LineTraceSingleByChannel(
        MuzzleHitResult,
        MuzzleLocation,
        MuzzleLocation + (DirectionToTarget * -500.0f),
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    FireFX();
    FireBullet(CameraHitResult, false);
}

void AMasterWeapon::Fire()
{
    // Early exits
    if (bReloading)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Fire] Blocked: bReloading = true"));
        return;
    }

    if (!WeaponSystem || !WeaponData)
    {
        UE_LOG(LogTemp, Error, TEXT("[Fire] WeaponSystem or WeaponData is NULL!"));
        return;
    }

    // Check if we have ammo
    if (!WeaponSystem->FireCheck(WeaponSystem->Weapon_Details.Weapon_Data.Ammo_Count))
    {
        // No ammo - handle empty fire
        if (WeaponSystem->Weapon_Details.Weapon_Data.MaxAmmo > 0)
        {
            if (bAutoReload)
            {
                Reload();
            }
            else if (WeaponData->EmptySound)
            {
                WeaponSystem->EmptyFX(WeaponData->EmptySound);
            }
        }
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[Fire] FireCheck PASSED - Executing fire logic"));

    // Get PlayerController and CameraManager
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->PlayerCameraManager)
    {
        UE_LOG(LogTemp, Error, TEXT("[Fire] PlayerController or CameraManager is NULL!"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[Fire] PlayerController found"));

    // Apply camera shake (only for players)
    ApplyCameraShake(PC);

    // Perform camera trace
    FHitResult CameraHitResult;
    if (PerformCameraTrace(PC->PlayerCameraManager, CameraHitResult))
    {
        // Hit something - fire at target
        ExecuteFireSequence(CameraHitResult);
    }
    else
    {
        // Didn't hit anything - fire blank tracer
        FireFX();
        FireBlankTracer();
    }
}

void AMasterWeapon::Reload()
{
    if (!WeaponSystem || !WeaponData)
        return;
    
    if (!WeaponSystem->CheckAmmo())
        return;
    bReloading = true;
    float ReloadDelay = 0.0f;
    if (WeaponSystem)
    {
        ReloadDelay = WeaponSystem->ReloadMontage(WeaponData->BodyReloadMontage);
    }

    if (WeaponMesh)
    {
        WeaponMesh->PlayAnimation(WeaponData->WeaponReloadMontage, false);
    }

    WeaponSystem->ReloadCheck();
    FTimerHandle ReloadTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        ReloadTimerHandle,
        [this]()
        {
            // Update UI only for players (not NPCs/Enemies)
            if (WeaponSystem && WeaponSystem->CharacterRef)
            {
                APlayer_Base* Player = Cast<APlayer_Base>(WeaponSystem->CharacterRef);
                if (Player && Player->CurrentWeaponUI)
                {
                    FWeapon_Details WeaponDetails = WeaponSystem->Weapon_Details;
                    Player->CurrentWeaponUI->UpdateAmmoCount(
                        WeaponDetails.Weapon_Data.MaxAmmo,
                        WeaponDetails.Weapon_Data.CurrentAmmo
                    );
                }
            }
            bReloading = false;
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
        EquipmentSys->Equip(TargetSlot, WeaponData);

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
    // For now, all interactions are instant
    // TODO: Implement Hold to swap later
    return false;
}

float AMasterWeapon::GetHoldDuration_Implementation() const
{
    // Hold duration for weapon swap
    return IsHoldInteraction_Implementation() ? 1.0f : 0.0f;
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
    if (WeaponMesh)
    {
        if (bHighlight)
        {
            // Enable custom depth for outline effect
            WeaponMesh->SetRenderCustomDepth(true);
            WeaponMesh->SetCustomDepthStencilValue(1);
        }
        else
        {
            // Disable custom depth
            WeaponMesh->SetRenderCustomDepth(false);
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

UInteractionData* AMasterWeapon::GetInteractionData_Implementation() const
{
    // Not using InteractionData - using direct interface methods instead
    return nullptr;
}

