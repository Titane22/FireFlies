// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/Equipments/GunAttackSystem.h"
#include "Gameplay/Items/Equipments/BulletTrace.h"
#include "Gameplay/Items/Equipments/MasterWeapon.h"
#include "Gameplay/Items/Equipments/MasterMagazine.h"
#include "Gameplay/Items/InventorySystem.h"
#include "Gameplay/Data/MagazineData.h"
#include "Gameplay/Characters/FFCharacter.h"
#include "Gameplay/Characters/Player_Base.h"
#include "Gameplay/Data/WeaponData.h"
#include "Presentations/HUD/PlayerHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Perception/AISense_Hearing.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UGunAttackSystem::UGunAttackSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGunAttackSystem::BeginPlay()
{
	Super::BeginPlay();

	// Owner 초기화
	OwnerWeapon = Cast<AMasterWeapon>(GetOwner());
	if (!OwnerWeapon)
	{
		UE_LOG(LogTemp, Error, TEXT("GunAttackSystem::BeginPlay - OwnerWeapon is NULL! Component must be attached to AMasterWeapon."));
		return;
	}
}

void UGunAttackSystem::OnAttackStarted()
{
	if (!CharacterRef || !CharacterRef->bIsAiming)
		return;

	bFiring = true;
	HandleFiring();
}

void UGunAttackSystem::OnAttackReleased()
{
	bFiring = false;
}

void UGunAttackSystem::HandleFiring()
{
	if (!bFiring || !bCanFire)
		return;

	if (!CharacterRef || !CharacterRef->CanAttack())
		return;

	ReadyToFire();
}

void UGunAttackSystem::ReadyToFire()
{
	if (!OwnerWeapon || !OwnerWeapon->WeaponData)
		return;

	bCanFire = false;
	PerformAttack();
	UpdateAmmoUI();

	float FireDelay = OwnerWeapon->WeaponData->FireRate;
	EFireMode CurrentFireMode = OwnerWeapon->WeaponData->FireMode;

	// 이전 타이머 정리
	if (GetWorld()->GetTimerManager().IsTimerActive(FireRateTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(FireRateTimerHandle);
	}

	switch (CurrentFireMode)
	{
	case EFireMode::FullAuto:
		GetWorld()->GetTimerManager().SetTimer(
			FireRateTimerHandle,
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
			FireRateTimerHandle,
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

void UGunAttackSystem::UpdateAmmoUI()
{
	if (!CharacterRef || !OwnerWeapon)
		return;

	APlayer_Base* Player = Cast<APlayer_Base>(CharacterRef);
	if (!Player || !Player->IsPlayerControlled())
		return;

	APlayerController* PC = Cast<APlayerController>(Player->GetController());
	if (!PC)
		return;

	APlayerHUD* HUD = Cast<APlayerHUD>(PC->GetHUD());
	if (HUD)
	{
		HUD->UpdateWeaponAmmo(
			OwnerWeapon->GetMaxAmmo(),
			OwnerWeapon->GetCurrentAmmo());
	}
}

void UGunAttackSystem::PerformAttack()
{
	// Early exits
	if (bReloading)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Fire] Blocked: bReloading = true"));
		return;
	}

	if (!OwnerWeapon || !OwnerWeapon->WeaponData)
	{
		UE_LOG(LogTemp, Error, TEXT("[Fire] OwnerWeapon or WeaponData is NULL!"));
		return;
	}

	// 탄창이 없으면 발사 불가
	if (!OwnerWeapon->CurrentMagazine)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Fire] No magazine loaded"));
		if (OwnerWeapon->WeaponData->EmptySound)
		{
			EmptyFX(OwnerWeapon->WeaponData->EmptySound);
		}
		return;
	}

	// Check if we have ammo
	if (!FireCheck(1/*TODO: 발사 탄 수*/))
	{
		// No ammo - handle empty fire
		if (bAutoReload && OwnerWeapon)
		{
			OwnerWeapon->Reload();
		}
		else if (OwnerWeapon->WeaponData->EmptySound)
		{
			EmptyFX(OwnerWeapon->WeaponData->EmptySound);
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
		PlayFireEffect();
		FireBlankTracer();
	}
}

bool UGunAttackSystem::CanReload() const
{
	// TODO: MaxAmmo를 지우고 ClipSize가 이전의 MaxAmmo를 대체?
	return CheckAmmo();
}

void UGunAttackSystem::ExecuteReload()
{
	// 탄약 계산만 수행 (애니메이션, 타이머, UI는 MasterWeapon에서 처리)
	ReloadCheck();
}

int32 UGunAttackSystem::GetCurrentAmmo() const
{
	if (!OwnerWeapon || !OwnerWeapon->CurrentMagazine)
		return 0;
	return OwnerWeapon->CurrentMagazine->GetCurrentAmmo();
}

int32 UGunAttackSystem::GetMaxAmmo() const
{
	if (!OwnerWeapon || !OwnerWeapon->CurrentMagazine)
		return 0;
	return OwnerWeapon->CurrentMagazine->GetClipSize();
}

void UGunAttackSystem::SetCurrentAmmo(float Amount)
{
	if (!OwnerWeapon || !OwnerWeapon->CurrentMagazine)
		return;
	// CurrentAmmo 직접 설정은 지양하지만, 호환성을 위해 유지
	OwnerWeapon->CurrentMagazine->CurrentAmmo = static_cast<int32>(Amount);
}

void UGunAttackSystem::FireBullet(FHitResult Hit, bool bReturnHit)
{
    if (!CharacterRef || !OwnerWeapon->WeaponData)
        return;

    for (int32 curBurst = 0; curBurst < OwnerWeapon->WeaponData->BurstAmount; curBurst++)
    {
        float PointX, PointY;
        RandPointInCircle(FMath::Tan(OwnerWeapon->WeaponData->BulletSpread) * 10.0f, PointX, PointY);

        APlayerCameraManager* CameraManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
        if (!CameraManager)
            return;

        FVector SpreadAdjustedHitLocation = Hit.Location + CameraManager->GetActorRightVector() * PointX + CameraManager->GetActorUpVector() * PointY;
        FVector MuzzleLocation = OwnerWeapon->EquipmentMesh->GetSocketLocation(FName("Muzzle"));
        FVector BulletDirection = (SpreadAdjustedHitLocation - MuzzleLocation).GetSafeNormal();

        // BulletTrace 스폰 - 충돌 시 데미지는 BulletTrace::ProcessHit에서 처리
        if (OwnerWeapon->WeaponData->BulletTraceClass)
        {
            FTransform SpawnTransform;
            SpawnTransform.SetLocation(MuzzleLocation);
            SpawnTransform.SetRotation(BulletDirection.Rotation().Quaternion());

            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = OwnerWeapon;
            SpawnParams.Instigator = CharacterRef;

            ABulletTrace* SpawnedBullet = GetWorld()->SpawnActor<ABulletTrace>(
                OwnerWeapon->WeaponData->BulletTraceClass, SpawnTransform, SpawnParams);
            if (SpawnedBullet)
            {
                SpawnedBullet->SetLifeSpan(BulletTraceLifeSpan);
                SpawnedBullet->BaseDamage = OwnerWeapon->WeaponData->Damage;
            }
        }
    }
}

void UGunAttackSystem::RandPointInCircle(float Radius, float& PointX, float& PointY)
{
	float Angle = FMath::RandRange(0.0f, 360.0f);
	float DistanceFromCenter = FMath::RandRange(0.0f, Radius);
    
	// Convert polar coordinates to cartesian coordinates
	PointX = DistanceFromCenter * FMath::Cos(FMath::DegreesToRadians(Angle));
	PointY = DistanceFromCenter * FMath::Sin(FMath::DegreesToRadians(Angle));
}

bool UGunAttackSystem::FireCheck(int32 AmmoCount)
{
	if (!OwnerWeapon || !OwnerWeapon->CurrentMagazine)
		return false;

	if (!OwnerWeapon->CurrentMagazine->HasAmmo())
		return false;

	// 탄약 소비
	int32 Consumed = OwnerWeapon->CurrentMagazine->ConsumeAmmo(AmmoCount);
	return Consumed > 0;
}

void UGunAttackSystem::FireFX(USoundBase* Sound, FVector Location, USoundAttenuation* AttenuationSettings, USoundConcurrency* ConcurrencySettings)
{
	if (!Sound)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::FireFX Sound is null"));
		return;
	}

	UGameplayStatics::SpawnSoundAtLocation(
		this,           // World context object
		Sound,          // Sound to play
		Location,       // Location to play sound at
		FRotator::ZeroRotator,  // Rotation (default to zero)
		1.0f,          // Volume multiplier
		1.0f,          // Pitch multiplier
		0.0f,          // Start time
		AttenuationSettings,    // Attenuation settings
		ConcurrencySettings     // Concurrency settings
	);

	UAISense_Hearing::ReportNoiseEvent(
		GetWorld(),
		Location,
		1.f,
		CharacterRef
	);
}

void UGunAttackSystem::EmptyFX(USoundBase* Sound)
{
	UGameplayStatics::PlaySoundAtLocation(
		this,
		Sound,
		CharacterRef->GetActorLocation()
	);
}

void UGunAttackSystem::MuzzleVFX(UNiagaraSystem* SystemTemplate, USceneComponent* AttachToComponent)
{
	if (!SystemTemplate || !AttachToComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("MuzzleVFX: SystemTemplate or AttachToComponent is null"));
		return;
	}

	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		SystemTemplate,           // 나이아가라 시스템 템플릿
		AttachToComponent,       // 부착할 컴포넌트
		NAME_None,               // 소켓 이름 (None으로 설정)
		FVector(0, 0, 0),        // 위치 오프셋
		FRotator(0, 0, 0),       // 회전 오프셋
		EAttachLocation::KeepRelativeOffset,  // 위치 타입
		true,                    // Auto Activate
		true,                    // Auto Destroy
		ENCPoolMethod::None,     // Pooling Method
		true                     // Pre Cull Check
	);

	if (!NiagaraComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("MuzzleVFX: Failed to spawn niagara component"));
	}
}

void UGunAttackSystem::FireBlankTracer()
{
	if (!OwnerWeapon->WeaponData || !OwnerWeapon->WeaponData->BulletTraceClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("MasterWeapon::FireBlankTracer - WeaponData or BulletTraceClass is NULL"));
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->PlayerCameraManager)
		return;

	// Apply camera shake (only for players)
	ApplyCameraShake(PC);

	FVector SocketLocation = OwnerWeapon->EquipmentMesh->GetSocketLocation(FName("Muzzle"));
	FVector TraceEndLocation = PC->PlayerCameraManager->GetRootComponent()->GetComponentLocation()
		+ PC->PlayerCameraManager->GetActorForwardVector() * 20000.0f;
	FVector DirectionVector = TraceEndLocation - SocketLocation;
	FRotator Rotation = UKismetMathLibrary::MakeRotFromX(DirectionVector);

	FTransform NewTransform(Rotation, SocketLocation, FVector(1.0f));

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerWeapon;
	SpawnParams.Instigator = CharacterRef;

	ABulletTrace* SpawnedBullet = GetWorld()->SpawnActor<ABulletTrace>(
		OwnerWeapon->WeaponData->BulletTraceClass, NewTransform, SpawnParams);
	if (SpawnedBullet)
	{
		SpawnedBullet->SetLifeSpan(BulletTraceLifeSpan);
		SpawnedBullet->BaseDamage = OwnerWeapon->WeaponData->Damage;
	}
}

void UGunAttackSystem::PlayFireEffect()
{
	if (!OwnerWeapon->WeaponData)
	{
		UE_LOG(LogTemp, Warning, TEXT("MasterWeapon::FireFX - WeaponData is NULL"));
		return;
	}

	if (!OwnerWeapon->WeaponData->SoundAttenuation)
	{
		UE_LOG(LogTemp, Warning, TEXT("MasterWeapon::FireFX - SoundAttenuation is NULL"));
		return;
	}

	if (!OwnerWeapon->WeaponData->SoundConcurrency)
	{
		UE_LOG(LogTemp, Warning, TEXT("MasterWeapon::FireFX - SoundConcurrency is NULL"));
		return;
	}

	FireFX(OwnerWeapon->WeaponData->FireSound,
		OwnerWeapon->EquipmentMesh->GetSocketLocation("Muzzle"),
		OwnerWeapon->WeaponData->SoundAttenuation,
		OwnerWeapon->WeaponData->SoundConcurrency);

	if (!OwnerWeapon->WeaponData->MuzzleFlashVFX)
	{
		UE_LOG(LogTemp, Warning, TEXT("MasterWeapon::FireFX - MuzzleFlashVFX is NULL"));
		return;
	}

	MuzzleVFX(OwnerWeapon->WeaponData->MuzzleFlashVFX, OwnerWeapon->Muzzle);

	FireMontage(OwnerWeapon->WeaponData->BodyFireMontage);

	OwnerWeapon->EquipmentMesh->PlayAnimation(OwnerWeapon->WeaponData->WeaponFireMontage, false);
}

void UGunAttackSystem::FireMontage(UAnimMontage* FireAnim)
{
	if (!CharacterRef)
		return;

	UAnimInstance* AnimInst = CharacterRef->GetMesh()->GetAnimInstance();
	if (!AnimInst)
		return;
	
	AnimInst->Montage_Play(FireAnim, 1.f);
}

bool UGunAttackSystem::CheckAmmo() const
{
	if (!OwnerWeapon || !OwnerWeapon->WeaponData || !CharacterRef)
		return false;

	// 현재 탄창이 가득 차 있으면 리로드 불필요
	if (OwnerWeapon->CurrentMagazine && OwnerWeapon->CurrentMagazine->IsFull())
		return false;

	// 인벤토리에서 호환 탄창 확인
	UInventorySystem* InventorySys = CharacterRef->FindComponentByClass<UInventorySystem>();
	if (!InventorySys)
		return false;

	return InventorySys->HasAmmo(OwnerWeapon->WeaponData->RequiredAmmoType);
}

float UGunAttackSystem::ReloadMontage(UAnimMontage* ReloadAnim)
{
	if (!CharacterRef || !ReloadAnim)
		return 0.0f;

	UAnimInstance* AnimInst = CharacterRef->GetMesh()->GetAnimInstance();
	if (!AnimInst)
		return 0.0f;

	return AnimInst->Montage_Play(ReloadAnim, 1.f);
}

void UGunAttackSystem::ReloadCheck()
{
	if (!OwnerWeapon || !OwnerWeapon->WeaponData || !CharacterRef)
		return;

	UInventorySystem* InventorySys = CharacterRef->FindComponentByClass<UInventorySystem>();
	if (!InventorySys)
		return;

	// 인벤토리에서 가장 탄약이 많은 탄창 검색
	FItemSlot* BestMagazineSlot = InventorySys->GetBestMagazineSlot(OwnerWeapon->WeaponData->RequiredAmmoType);
	if (!BestMagazineSlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReloadCheck: No compatible magazine found in inventory"));
		return;
	}

	UMagazineData* NewMagazineData = Cast<UMagazineData>(BestMagazineSlot->ItemData.Get());
	if (!NewMagazineData)
		return;

	// 새 탄창 생성 
	UMasterMagazine* NewMagazine = NewObject<UMasterMagazine>(OwnerWeapon);
	NewMagazine->InitializeFromData(NewMagazineData, BestMagazineSlot->CurrentAmmo);

	// 탄창 교체 (이전 탄창 반환)
	UMasterMagazine* OldMagazine = OwnerWeapon->SwapMagazine(NewMagazine);

	// 기존 탄창 처리 (있으면 인벤토리로 복귀)
	if (OldMagazine && OldMagazine->MagazineData)
	{
		bool bReturned = InventorySys->AddMagazine(OldMagazine->MagazineData, OldMagazine->GetCurrentAmmo());
		if (bReturned)
		{
			UE_LOG(LogTemp, Log, TEXT("ReloadCheck: Old magazine returned to inventory (Ammo: %d)"),
				OldMagazine->GetCurrentAmmo());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ReloadCheck: Failed to return old magazine to inventory (no space?)"));
			// 인벤토리가 가득 찬 경우 - 탄창 드롭 로직 필요 (TODO)
		}
		// OldMagazine은 GC가 자동 처리
	}

	// 인벤토리에서 사용한 탄창 슬롯 제거
	InventorySys->RemoveItem(BestMagazineSlot->InstanceId);

	UE_LOG(LogTemp, Log, TEXT("ReloadCheck: Magazine swapped (New ammo: %d/%d)"),
		NewMagazine->GetCurrentAmmo(), NewMagazine->GetClipSize());
}

bool UGunAttackSystem::PerformCameraTrace(APlayerCameraManager* CameraManager, FHitResult& OutHitResult)
{
    if (!CameraManager || !OwnerWeapon->WeaponData)
        return false;
    APlayer_Base* Player = Cast<APlayer_Base>(CharacterRef);
    if (!Player)
        return false;

    APlayerCameraManager* PCM = Player->GetPlayerCameraManager();
    if (!PCM)
        return false;
    
    FVector StartLocation = PCM->GetCameraLocation();
    FVector ForwardVector = PCM->GetActorForwardVector();
    FVector EndLocation = StartLocation + (ForwardVector * OwnerWeapon->WeaponData->MaxRange);
    UE_LOG(LogTemp, Warning, TEXT("[PerformCameraTrace] StartLocation: %s, ForwardVector: %s, EndLocation: %s"),
                *StartLocation.ToString(), *ForwardVector.ToString(), *EndLocation.ToString());
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerWeapon);

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

void UGunAttackSystem::ApplyCameraShake(APlayerController* PC)
{
	// Only apply camera shake for player-controlled characters
	if (!PC || !CharacterRef)
		return;

	APlayer_Base* Player = Cast<APlayer_Base>(CharacterRef);
	if (Player)
	{
		// TODO: PC->ClientStartCameraShake(UWeaponFireCameraShake::StaticClass(), 1.0f);
	}
}

void UGunAttackSystem::ExecuteFireSequence(const FHitResult& CameraHitResult)
{
    PlayFireEffect();
    FireBullet(CameraHitResult, false);
}
