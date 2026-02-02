// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Combat/CreatureMeleeSystem.h"

#include "Components/CapsuleComponent.h"
#include "Gameplay/Characters/FFCharacter.h"
#include "Gameplay/Interfaces/Damageable.h"
#include "Kismet/KismetSystemLibrary.h"

UCreatureMeleeSystem::UCreatureMeleeSystem()
{
}

void UCreatureMeleeSystem::BeginPlay()
{
	// Skip UMeleeAttackSystem::BeginPlay which requires OwnerWeapon
	UWeaponAttackSystem::BeginPlay();

	CharacterRef = Cast<AFFCharacter>(GetOwner());
	if (!CharacterRef)
	{
		UE_LOG(LogTemp, Error, TEXT("CreatureMeleeSystem::BeginPlay - Owner is not an AFFCharacter!"));
	}
}

void UCreatureMeleeSystem::StartWeaponSweep()
{
	if (!CharacterRef)
		return;

	bWeaponSweepActive = true;
	TargetActors.Empty();
	SetComponentTickEnabled(false);

	if (CreatureComboAttacks.IsValidIndex(CurrentAttackIndex))
	{
		WeaponSweepStartSocket = CreatureComboAttacks[CurrentAttackIndex].SweepStartSocket;
		WeaponSweepEndSocket = CreatureComboAttacks[CurrentAttackIndex].SweepEndSocket;
	}
	WeaponRadius = CreatureHitRadius;
}

bool UCreatureMeleeSystem::GetWeaponTracePoints(FVector& OutStart, FVector& OutEnd) const
{
	if (!CharacterRef)
		return false;

	USkeletalMeshComponent* Mesh = CharacterRef->GetMesh();
	if (!Mesh)
		return false;

	if (!Mesh->DoesSocketExist(WeaponSweepStartSocket) || !Mesh->DoesSocketExist(WeaponSweepEndSocket))
		return false;

	OutStart = Mesh->GetSocketLocation(WeaponSweepStartSocket);
	OutEnd = Mesh->GetSocketLocation(WeaponSweepEndSocket);
	return true;
}

bool UCreatureMeleeSystem::OnWeaponHit(AActor* HitActor)
{
	if (!HitActor || !CharacterRef)
		return false;

	if (TargetActors.Contains(HitActor))
		return false;

	if (!HitActor->Implements<UDamageable>())
		return false;

	TargetActors.Add(HitActor);

	FVector Direction = (HitActor->GetActorLocation() - CharacterRef->GetActorLocation()).GetSafeNormal();

	FHitResult HitResult;
	HitResult.ImpactPoint = HitActor->GetActorLocation();
	HitResult.Location = HitActor->GetActorLocation();
	HitResult.ImpactNormal = -Direction;
	HitResult.HitObjectHandle = FActorInstanceHandle(HitActor);

	float Damage = CurrentAttack.DamageMultiplier * BaseDamage;
	bool bValidHit;
	ApplyHit(HitResult, Damage, bValidHit);

	return true;
}

void UCreatureMeleeSystem::InitializeComboAttacks()
{
	ComboAttacks.Reset(CreatureComboAttacks.Num());
	for (const FCreatureComboAttack& Attack : CreatureComboAttacks)
	{
		ComboAttacks.Add(Attack);
	}
}

void UCreatureMeleeSystem::CauseDamage()
{
	if (!CharacterRef)
		return;

	USkeletalMeshComponent* Mesh = CharacterRef->GetMesh();
	if (!Mesh)
		return;

	FVector HitLocation = Mesh->GetSocketLocation(FName("HitLocation"));
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(CharacterRef);

	TArray<AActor*> OverlappedActors;
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		HitLocation,
		CreatureHitRadius,
		ObjectTypeQuery,
		nullptr,
		IgnoreActors,
		OverlappedActors
	);

	bool bValidHit = false;
	for (AActor* CurrentActor : OverlappedActors)
	{
		float Damage = CurrentAttack.DamageMultiplier * BaseDamage;
		FHitResult HitResult;
		HitResult.bBlockingHit = true;
		HitResult.Location = HitLocation;

		bool bIsDead = ApplyHit(HitResult, Damage, bValidHit);
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
