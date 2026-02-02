// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/Equipments/MeleeAttackSystem.h"

#include "Components/CapsuleComponent.h"
#include "Gameplay/Characters/FFCharacter.h"
#include "Gameplay/Items/Equipments/MasterWeapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MotionWarpingComponent.h"

UMeleeAttackSystem::UMeleeAttackSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UMeleeAttackSystem::BeginPlay()
{
	Super::BeginPlay();

	OwnerWeapon = Cast<AMasterWeapon>(GetOwner());
	if (!OwnerWeapon)
	{
		UE_LOG(LogTemp, Error, TEXT("MeleeAttackSystem::BeginPlay - OwnerWeapon is NULL!"));
		return;
	}
}

void UMeleeAttackSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (CharacterRef)
	{
		CalculateTargetYaw();
		float NewYaw = FMath::RInterpTo(
			FRotator(0.f, CharacterRef->GetActorRotation().Yaw, 0.f),
			FRotator(0.f, TargetYaw, 0.f),
			GetWorld()->GetDeltaSeconds(),
			8.f).Yaw;

		CharacterRef->SetActorRotation(
			FRotator(0.f, NewYaw, 0.f));
	}
}

void UMeleeAttackSystem::PerformAttack()
{
	if (!CharacterRef)
		return;

	if (bCanCombo)
	{
		bAttackQueued = true;
		return;
	}

	// 콤보가 진행 중인데 콤보 윈도우가 안 열렸으면 입력 무시
	if (CurrentAttackIndex >= 0)
		return;

	if (!CanAttack())
		return;

	GetNextAttack();
	PlayCurrentAttack();
}

void UMeleeAttackSystem::PerformComboAttack(int32 NumAttacks)
{
	RemainingAutoAttacks = FMath::Max(0, NumAttacks - 1);
	PerformAttack();
}

void UMeleeAttackSystem::PlayCurrentAttack()
{
	if (!CharacterRef)
		return;

	if (TargetActor)
	{
		if (UMotionWarpingComponent* MWC = CharacterRef->FindComponentByClass<UMotionWarpingComponent>())
		{
			FVector ToTarget = TargetActor->GetActorLocation() - CharacterRef->GetActorLocation();
			FVector Direction = ToTarget.GetSafeNormal();
			FVector WarpLocation = TargetActor->GetActorLocation() - Direction * WarpTargetOffset;

			FMotionWarpingTarget WarpTarget;
			WarpTarget.Name = FName("AttackTarget");
			WarpTarget.Location = WarpLocation;
			WarpTarget.Rotation = Direction.Rotation();
			MWC->AddOrUpdateWarpTarget(WarpTarget);
		}
	}

	if (USkeletalMeshComponent* CharMesh = CharacterRef->GetMesh())
	{
		if (UAnimInstance* Anim = CharMesh->GetAnimInstance())
		{
			Anim->Montage_Play(CurrentAttack.AnimMontage, CurrentAttack.AttackSpeedRate);
		}
	}
}

void UMeleeAttackSystem::ExecuteQueuedAttack()
{
	bAttackQueued = false;
	bCanCombo = false;
	bIsExecutingQueued = true;

	GetNextAttack();
	PlayCurrentAttack();

	bIsExecutingQueued = false;
}

void UMeleeAttackSystem::SetComboState(EMeleeAttackType State, bool bEnable)
{
	switch (State)
	{
	case EMeleeAttackType::CanStartNextAttack:
		bCanCombo = bEnable;
		if (bEnable && RemainingAutoAttacks > 0)
		{
			bAttackQueued = true;
			--RemainingAutoAttacks;
		}
		if (!bEnable && !bIsExecutingQueued)
		{
			if (bAttackQueued)
			{
				ExecuteQueuedAttack();
			}
			else
			{
				ResetCombo();
			}
		}
		break;

	case EMeleeAttackType::CanInterrupt:
		bCanBeInterrupted = bEnable;
		break;

	case EMeleeAttackType::EndCombo:
		if (bEnable)
		{
			ResetCombo();
		}
		break;

	default:
		break;
	}
}

void UMeleeAttackSystem::ResetCombo()
{
	CurrentAttackIndex = -1;
	bCanCombo = false;
	bAttackQueued = false;
	bCanBeInterrupted = false;
	RemainingAutoAttacks = 0;
	SetComponentTickEnabled(false);
	TargetActor = nullptr;
	OnComboEnded.Broadcast();
}

bool UMeleeAttackSystem::TryInterruptAttack()
{
	if (!bCanBeInterrupted)
		return false;

	if (CharacterRef)
	{
		if (UAnimInstance* Anim = CharacterRef->GetMesh()->GetAnimInstance())
		{
			Anim->Montage_Stop(0.25f);
		}
	}

	StopWeaponSweep();
	ResetCombo();
	return true;
}

void UMeleeAttackSystem::StartWeaponSweep()
{
	if (!CharacterRef || !OwnerWeapon)
		return;
	bWeaponSweepActive = true;
	TargetActors.Empty();
	SetComponentTickEnabled(false);
	 
	if (OwnerWeapon->WeaponData)
	{
		WeaponSweepStartSocket = OwnerWeapon->WeaponData->SweepStartSocket;
		WeaponSweepEndSocket = OwnerWeapon->WeaponData->SweepEndSocket;
		WeaponRadius = OwnerWeapon->WeaponData->HitRadius;;
	}
}

void UMeleeAttackSystem::StopWeaponSweep()
{
	bWeaponSweepActive = false;
	TargetActors.Empty();
	TargetActor = nullptr;
}

float UMeleeAttackSystem::GetWeaponSweepRadius()
{
	return WeaponRadius;
}

bool UMeleeAttackSystem::CanAttack()
{
	if (!CharacterRef)
		return false;
	return CharacterRef->CanAttack();
}

void UMeleeAttackSystem::GetNextAttack()
{
	if (!CharacterRef)
		return;

	if (CharacterRef->bIsAiming)
		return;
	InitializeComboAttacks();
	CheckTarget();

	++CurrentAttackIndex;
	if (CurrentAttackIndex >= ComboAttacks.Num())
	{
		CurrentAttackIndex = 0;
	}

	CurrentAttack = ComboAttacks[CurrentAttackIndex];
}

void UMeleeAttackSystem::CheckTarget()
{
	Distance = 999999.f;
	TargetActor = nullptr;

	FVector SpherePos = CharacterRef->GetActorLocation();
	float SphereRadius = CheckTargetsRadius;

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(CharacterRef);

	TArray<AActor*> OverlappedActors;
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		SpherePos,
		SphereRadius,
		ObjectTypeQuery,
		nullptr,
		IgnoreActors,
		OverlappedActors);

	for (AActor* CurrentActor : OverlappedActors)
	{
		if (!CurrentActor->Implements<UDamageable>())
			continue;

		float CurrentDistance = FVector::Dist(CharacterRef->GetActorLocation(), CurrentActor->GetActorLocation());
		if (CurrentDistance < Distance)
		{
			Distance = CurrentDistance;
			TargetActor = CurrentActor;
		}
	}
	if (TargetActor)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, TEXT("SetComponentTickEnabled(true)"));
		SetComponentTickEnabled(true);
	}
}

void UMeleeAttackSystem::CalculateTargetYaw()
{
	if (!CharacterRef)
		return;
	if (TargetActor)
	{
		TargetYaw = UKismetMathLibrary::FindLookAtRotation(CharacterRef->GetActorLocation(), TargetActor->GetActorLocation()).Yaw;
	}
	else
	{
		TargetYaw = CharacterRef->GetControlRotation().Yaw;
	}
}

void UMeleeAttackSystem::InitializeComboAttacks()
{
	if (!OwnerWeapon || !OwnerWeapon->WeaponData)
		return;

	ComboAttacks = OwnerWeapon->WeaponData->ComboAttacks;
}

void UMeleeAttackSystem::CauseDamage()
{
	if (!OwnerWeapon || !OwnerWeapon->EquipmentStaticMesh)
		return;

	
	FVector HitLocation = OwnerWeapon->EquipmentStaticMesh->GetSocketLocation(FName("HitLocation"));
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(CharacterRef);

	TArray<AActor*> OverlappedActors;
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		HitLocation,
		OwnerWeapon->WeaponData->HitRadius,
		ObjectTypeQuery,
		nullptr,
		IgnoreActors,
		OverlappedActors
	);
	bool bValidHit;
	for (AActor* CurrentActor : OverlappedActors)
	{
		float BaseDamage = CurrentAttack.DamageMultiplier * OwnerWeapon->WeaponData->Damage;
		FHitResult HitResult;
		HitResult.bBlockingHit = true;
		HitResult.Location = HitLocation;

		bool bIsDead = ApplyHit(HitResult, BaseDamage, bValidHit);
	}
	
	if (bValidHit)
	{
		float CapHeight, CapWidth;
		CharacterRef->GetCapsuleComponent()->GetScaledCapsuleSize(CapWidth, CapHeight);
		float StartX = CharacterRef->GetMesh()->GetComponentLocation().X;
		float StartY = CharacterRef->GetMesh()->GetComponentLocation().Y;
		float StartZ = CharacterRef->GetMesh()->GetComponentLocation().Z + (CapHeight * 2.f);

		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			FVector(StartX, StartY, StartZ),
			HitLocation,
			ECC_Visibility
		);

		if (bHit)
		{
			HitLocation = HitResult.ImpactPoint;
		}

		// TODO: Spawn Sound

		// TODO: Spawn Effect
	}
}

bool UMeleeAttackSystem::GetWeaponTracePoints(FVector& OutStart, FVector& OutEnd) const
{
	if (!OwnerWeapon || !OwnerWeapon->EquipmentStaticMesh)
		return false;

	if (!OwnerWeapon->EquipmentStaticMesh->DoesSocketExist(WeaponSweepStartSocket) || !OwnerWeapon->EquipmentStaticMesh->DoesSocketExist(WeaponSweepEndSocket))
		return false;

	OutStart = OwnerWeapon->EquipmentStaticMesh->GetSocketLocation(WeaponSweepStartSocket);
	OutEnd = OwnerWeapon->EquipmentStaticMesh->GetSocketLocation(WeaponSweepEndSocket);
	return true;
}

bool UMeleeAttackSystem::OnWeaponHit(AActor* HitActor)
{
	if (!HitActor || !OwnerWeapon || !OwnerWeapon->WeaponData || !CharacterRef)
		return false;
	// 이미 이번 스윙에서 히트한 액터는 무시
	if (TargetActors.Contains(HitActor))
		return false;

	if (!HitActor->Implements<UDamageable>())
		return false;

	TargetActors.Add(HitActor);
	
	// 공격 방향: 공격자 → 피격자
	FVector Direction = (HitActor->GetActorLocation() - CharacterRef->GetActorLocation()).GetSafeNormal();

	FHitResult HitResult;
	HitResult.ImpactPoint = HitActor->GetActorLocation();
	HitResult.Location = HitActor->GetActorLocation();
	HitResult.ImpactNormal = -Direction;
	HitResult.HitObjectHandle = FActorInstanceHandle(HitActor);

	float BaseDamage = CurrentAttack.DamageMultiplier * OwnerWeapon->WeaponData->Damage;
	bool bValidHit;
	ApplyHit(HitResult, BaseDamage, bValidHit);

	return true;
}
