// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/Equipments/MeleeAttackSystem.h"

#include "Components/CapsuleComponent.h"
#include "Gameplay/Characters/FFCharacter.h"
#include "Gameplay/Items/Equipments/MasterWeapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

UMeleeAttackSystem::UMeleeAttackSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
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

		float NewRotationZ = FMath::RInterpTo(
			FRotator(0.f,0.f,CharacterRef->GetActorRotation().Yaw),
			FRotator(0.f, 0.f, TargetYaw),
			GetWorld()->GetDeltaSeconds(),
			8.f).Yaw;
		
		CharacterRef->SetActorRotation(
			FRotator(
				0.f,
				0.f,
				NewRotationZ));
	}
}

void UMeleeAttackSystem::PerformAttack()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, TEXT("Called"));
	if (!CharacterRef)
		return;
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, TEXT("Called"));

	if (bCanCombo)
	{
		bAttackQueued = true;
		return;
	}
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Called"));

	if (!CanAttack())
		return;
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, TEXT("Called"));

	GetNextAttack();
	PlayCurrentAttack();
}

void UMeleeAttackSystem::PlayCurrentAttack()
{
	if (!CharacterRef)
		return;

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

	GetNextAttack();
	PlayCurrentAttack();
}

void UMeleeAttackSystem::SetComboState(EMeleeAttackType State, bool bEnable)
{
	switch (State)
	{
	case EMeleeAttackType::CanStartNextAttack:
		bCanCombo = bEnable;
		if (!bEnable)
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
	SetComponentTickEnabled(false);
	TargetActor = nullptr;
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

void UMeleeAttackSystem::OnWeaponHit(FHitResult& HitResult)
{
	if (!OwnerWeapon || !OwnerWeapon->WeaponData)
		return;
	AActor* HitActor = HitResult.GetActor();
	if (!HitActor)
		return;

	if (HitActor->Implements<UDamageable>())
	{
		bool bValidHit;
		bool bIsDead = ApplyHit(HitResult, OwnerWeapon->WeaponData->Damage, bValidHit);
	}
}
