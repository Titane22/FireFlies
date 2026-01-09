// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Items/Equipments/MasterWeapon.h"
#include "Gameplay/Items/Equipments/WeaponSystem.h"
#include "Gameplay/Characters/FFCharacter.h"
#include "Gameplay/Characters/Player_Base.h"
//#include "Gameplay/Items/Equipments/WeaponFireCameraShake.h"
#include "Gameplay/Data/WeaponData.h"
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
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
    WeaponSystem = CreateDefaultSubobject<UWeaponSystem>(TEXT("WeaponSystem"));

    // Setting up the component hierarchy
    RootComponent = DefaultSceneRoot;
    WeaponMesh->SetupAttachment(DefaultSceneRoot);
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
}

void AMasterWeapon::FireBullet(FHitResult Hit, bool bReturnHit)
{
    // ===== 디버그 로그 추가 =====
    UE_LOG(LogTemp, Warning, TEXT("[FireBullet] Hit.Location: %s, bBlockingHit: %d"),
        *Hit.Location.ToString(), Hit.bBlockingHit);
    // ===========================

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

        // ===== 디버그 로그 추가 =====
        UE_LOG(LogTemp, Warning, TEXT("[FireBullet] MuzzleLocation: %s"), *MuzzleLocation.ToString());
        UE_LOG(LogTemp, Warning, TEXT("[FireBullet] SpreadAdjustedHitLocation: %s"), *SpreadAdjustedHitLocation.ToString());
        // ===========================
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

    UCameraComponent* Cam = WeaponSystem->CharacterRef->FindComponentByClass<UCameraComponent>();
    if (!Cam)
        return false;
    
    FVector StartLocation = Cam->GetComponentLocation();
    FVector ForwardVector = Cam->GetForwardVector();
    FVector EndLocation = StartLocation + (ForwardVector * WeaponData->MaxRange);
    UE_LOG(LogTemp, Warning, TEXT("[PerformCameraTrace] StartLocation: %s, ForwardVector: %s, EndLocation: %s"),
                *StartLocation.ToString(), *ForwardVector.ToString(), *EndLocation.ToString());
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

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
        COLLISION_BULLET,
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

    UE_LOG(LogTemp, Log, TEXT("[Fire] Called - CurrentAmmo: %d, AmmoCount: %d"),
        WeaponSystem->Weapon_Details.Weapon_Data.CurrentAmmo,
        WeaponSystem->Weapon_Details.Weapon_Data.Ammo_Count);

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

