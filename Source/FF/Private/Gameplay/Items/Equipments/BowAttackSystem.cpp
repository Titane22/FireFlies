// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/Equipments/BowAttackSystem.h"

#include "Gameplay/Animations/ABP_Bow.h"
#include "Gameplay/Items/Equipments/MasterWeapon.h"
#include "Gameplay/Items/InventorySystem.h"
#include "Gameplay/Items/Projectiles/MasterProjectile.h"
#include "Gameplay/Characters/FFCharacter.h"
#include "Gameplay/Characters/Player_Base.h"
#include "Gameplay/Data/WeaponData.h"
#include "Gameplay/Data/ItemData.h"

UBowAttackSystem::UBowAttackSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBowAttackSystem::BeginPlay()
{
	Super::BeginPlay();

	// Owner 초기화
	OwnerWeapon = Cast<AMasterWeapon>(GetOwner());
	if (!OwnerWeapon)
	{
		UE_LOG(LogTemp, Error, TEXT("BowAttackSystem::BeginPlay - OwnerWeapon is NULL!"));
		return;
	}
}

void UBowAttackSystem::PerformAttack()
{
	// 기존 즉발 발사: 차지 없이 기본 파워로 발사
	if (!HasArrowEquipped())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BowAttackSystem] PerformAttack: No arrow equipped"));

		// 자동 장전이 켜져있으면 화살 스폰 시도
		if (bAutoReload)
		{
			SpawnArrow();
		}
		return;
	}

	FireArrow(1.0f);
}

bool UBowAttackSystem::CanReload() const
{
	if (!CharacterRef || !OwnerWeapon || !OwnerWeapon->WeaponData)
		return false;

	// 이미 화살이 장착되어 있으면 리로드 불필요
	if (SpawnedArrow)
		return false;

	// 인벤토리에 화살이 있는지 확인
	UInventorySystem* InventorySys = CharacterRef->FindComponentByClass<UInventorySystem>();
	if (!InventorySys)
		return false;

	return InventorySys->HasArrow(OwnerWeapon->WeaponData->RequiredAmmoType);
}

void UBowAttackSystem::ExecuteReload()
{
	
}

void UBowAttackSystem::SpawnArrow()
{
	if (!CharacterRef || !OwnerWeapon || !OwnerWeapon->WeaponData)
	{
		UE_LOG(LogTemp, Error, TEXT("[BowAttackSystem] SpawnArrow: Invalid references"));
		return;
	}

	// 이미 화살이 장착되어 있으면 스킵
	if (SpawnedArrow)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BowAttackSystem] SpawnArrow: Arrow already equipped"));
		return;
	}

	// 인벤토리에서 화살 확인
	UInventorySystem* InventorySys = CharacterRef->FindComponentByClass<UInventorySystem>();
	if (!InventorySys)
	{
		UE_LOG(LogTemp, Error, TEXT("[BowAttackSystem] SpawnArrow: No inventory system"));
		return;
	}

	FGameplayTag RequiredArrowTag = OwnerWeapon->WeaponData->RequiredAmmoType;

	// 화살이 없으면 스폰 불가
	if (!InventorySys->HasArrow(RequiredArrowTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("[BowAttackSystem] SpawnArrow: No arrows in inventory"));
		return;
	}

	// 화살 슬롯에서 ItemData 가져오기 (아직 소모하지 않음)
	FItemSlot* ArrowSlot = InventorySys->GetBestArrowSlot(RequiredArrowTag);
	if (!ArrowSlot)
	{
		UE_LOG(LogTemp, Error, TEXT("[BowAttackSystem] SpawnArrow: Failed to get arrow slot"));
		return;
	}

	CurrentArrowData = ArrowSlot->ItemData.Get();

	// 화살 액터 클래스 확인
	TSubclassOf<AActor> ArrowClass = OwnerWeapon->WeaponData->BulletTraceClass;

	// ArrowActorClass가 설정되지 않았으면 ItemData의 EquipmentClass 사용
	if (!ArrowClass && CurrentArrowData)
	{
		ArrowClass = CurrentArrowData->EquipmentClass;
	}

	if (!ArrowClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[BowAttackSystem] SpawnArrow: No arrow class specified"));
		return;
	}

	// 캐릭터 메쉬의 arrow_socket에 화살 스폰
	USkeletalMeshComponent* CharMesh = CharacterRef->GetMesh();
	if (!CharMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("[BowAttackSystem] SpawnArrow: Character mesh is NULL"));
		return;
	}

	// 소켓 위치에서 스폰
	FVector SocketLocation = CharMesh->GetSocketLocation(ArrowSocketName);
	FRotator SocketRotation = CharMesh->GetSocketRotation(ArrowSocketName);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = CharacterRef;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SpawnedArrow = GetWorld()->SpawnActor<AMasterProjectile>(
		ArrowClass,
		SocketLocation,
		SocketRotation,
		SpawnParams
	);

	if (SpawnedArrow)
	{
		// 소켓에 부착
		SpawnedArrow->AttachToComponent(
			CharMesh,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			ArrowSocketName
		);

		UE_LOG(LogTemp, Log, TEXT("[BowAttackSystem] SpawnArrow: Arrow spawned and attached to %s"),
			*ArrowSocketName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[BowAttackSystem] SpawnArrow: Failed to spawn arrow actor"));
	}
}

void UBowAttackSystem::DestroySpawnedArrow()
{
	if (SpawnedArrow)
	{
		SpawnedArrow->Destroy();
		SpawnedArrow = nullptr;
		CurrentArrowData = nullptr;

		UE_LOG(LogTemp, Log, TEXT("[BowAttackSystem] DestroySpawnedArrow: Arrow destroyed"));
	}
}

void UBowAttackSystem::FireArrow(float ChargeRatio)
{
	if (!SpawnedArrow || !CharacterRef || !OwnerWeapon || !OwnerWeapon->WeaponData)
	{
		UE_LOG(LogTemp, Error, TEXT("[BowAttackSystem] FireArrow: Invalid state"));
		return;
	}
	// 인벤토리에서 화살 1개 소모
	UInventorySystem* InventorySys = CharacterRef->FindComponentByClass<UInventorySystem>();
	if (InventorySys)
	{
		UItemData* ConsumedArrow = InventorySys->ConsumeArrow(OwnerWeapon->WeaponData->RequiredAmmoType);
		if (!ConsumedArrow)
		{
			UE_LOG(LogTemp, Warning, TEXT("[BowAttackSystem] FireArrow: Failed to consume arrow from inventory"));
		}
	}

	// 화살 발사 (Detach하고 물리/투사체 활성화)
	if (SpawnedArrow)
	{
		FVector AimDirection;
		APlayer_Base* Player = Cast<APlayer_Base>(CharacterRef);
		if (Player)
		{
			APlayerCameraManager* PCM = Player->GetPlayerCameraManager();
			if (PCM)
			{
				FVector CamLocation = PCM->GetCameraLocation();
				FVector CamForward = PCM->GetActorForwardVector();
				FVector TraceEnd = CamLocation + (CamForward * 10000.f);

				FHitResult Hit;
				FCollisionQueryParams CollisionParams;
				CollisionParams.AddIgnoredActor(CharacterRef);
				CollisionParams.AddIgnoredActor(SpawnedArrow);

				FVector TargetPoint = TraceEnd;
				if (GetWorld()->LineTraceSingleByChannel(Hit, CamLocation, TraceEnd, ECC_Visibility, CollisionParams))
				{
					TargetPoint = Hit.ImpactPoint;
				}

				AimDirection = (TargetPoint - SpawnedArrow->GetActorLocation()).GetSafeNormal();
			}
			else
			{
				AimDirection = CharacterRef->GetActorForwardVector();
			}
		}
		else
		{
			AimDirection = CharacterRef->GetActorForwardVector();
		}
		// 부착 해제
		SpawnedArrow->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		SpawnedArrow->Fire(AimDirection, ChargeRatio);
		
		UE_LOG(LogTemp, Log, TEXT("[BowAttackSystem] FireArrow: Arrow fired"));

		// 발사 후 레퍼런스 초기화 (화살은 독립적으로 날아감)
		SpawnedArrow = nullptr;
		CurrentArrowData = nullptr;
	}

	// 자동 재장전이 켜져있으면 다음 화살 스폰
	if (bAutoReload && CanReload())
	{
		SpawnArrow();
	}
}

void UBowAttackSystem::StartCharge()
{
	if (!CharacterRef || !OwnerWeapon)
		return;

	if (bIsCharging)
		return;

	// 화살이 없으면 먼저 장착 시도
	if (!HasArrowEquipped())
	{
		SpawnArrow();

		// 스폰 실패 시 차지 불가
		if (!HasArrowEquipped())
			return;
	}

	bIsCharging = true;
	ChargeStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	// 0.0 ~ 1.0 범위로 정규화 (MaxChargeTime 이상은 1.0으로 클램프)
	
	const float Now = GetWorld()->GetTimeSeconds();
	const float HeldTime = FMath::Max(0.0f, Now - ChargeStartTime);
	const float ChargeRatio = MaxChargeTime > 0.0f
			? FMath::Clamp(HeldTime / MaxChargeTime, 0.0f, 1.0f)
			: 1.0f;
	// TODO: 현재 이 함수는 한 번 호출되므로 타임라인으로 빼서 값을 주는 것으로 하는게 좋을듯
	if (OwnerWeapon && OwnerWeapon->EquipmentMesh)
	{
		if (UAnimInstance* AnimInst = OwnerWeapon->EquipmentMesh->GetAnimInstance())
		{
			if (UABP_Bow* BowAnimInstance = Cast<UABP_Bow>(AnimInst))
			{
				BowAnimInstance->DrawBow = 1.f;
			}
		}
	}
}

void UBowAttackSystem::ReleaseCharge()
{
	if (!bIsCharging)
		return;

	bIsCharging = false;

	if (!GetWorld())
		return;

	// 애니메이션 인스턴스에 비율 전달
	if (OwnerWeapon && OwnerWeapon->EquipmentMesh)
	{
		if (UAnimInstance* AnimInst = OwnerWeapon->EquipmentMesh->GetAnimInstance())
		{
			if (UABP_Bow* BowAnimInstance = Cast<UABP_Bow>(AnimInst))
			{
				BowAnimInstance->DrawBow = 0.f;
			}
		}
	}
	const float Now = GetWorld()->GetTimeSeconds();
	const float HeldTime = FMath::Max(0.0f, Now - ChargeStartTime);
	const float ChargeRatio = MaxChargeTime > 0.0f
			? FMath::Clamp(HeldTime / MaxChargeTime, 0.0f, 1.0f)
			: 1.0f;
	// 화살 발사
	FireArrow(ChargeRatio);
}
