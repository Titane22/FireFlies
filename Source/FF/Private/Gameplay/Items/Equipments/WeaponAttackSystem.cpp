// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/Equipments/WeaponAttackSystem.h"

#include "Engine/DamageEvents.h"
#include "Gameplay/Characters/FFCharacter.h"
#include "Gameplay/Interfaces/Damageable.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UWeaponAttackSystem::UWeaponAttackSystem()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UWeaponAttackSystem::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UWeaponAttackSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


bool UWeaponAttackSystem::ApplyHit(const FHitResult HitResult, float DamageAmount, bool& ValidHit)
{
	AActor* HitActor = HitResult.GetActor();
    
	// Hit된 액터가 있고, 그 액터가 유효한지 확인
	if (!HitActor)
	{
		ValidHit = false;
		return false;
	}
    
	bool bIsDead = false;
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow,
			FString::Printf(TEXT("Hit Actor: %s"), HitActor ? *HitActor->GetName() : TEXT("NULL")));
	if (HitActor->Implements<UDamageable>())
	{
		FPointDamageEvent DamageEvent(
			  DamageAmount,           // Damage
			  HitResult,                    // HitInfo
			  -HitResult.ImpactNormal,      // ShotDirection (총알 방향)
			  nullptr                       // DamageTypeClass
		  );

		APawn* OwnerPawn = CharacterRef;
		AController* OwnerController = OwnerPawn ? OwnerPawn->GetController() : nullptr;

		float ActualDamage = IDamageable::Execute_TakeDamage(
			HitActor,
			DamageAmount,
			DamageEvent,
			HitResult.BoneName,
			OwnerController,
			GetOwner()
		);
		ValidHit = ActualDamage > 0.0f;
		// TODO: 호출시 사용
		// if (ValidHit)
		// {
		// 	if (WeaponData && WeaponData->HitMarkerSound)
		// 	{
		// 		UGameplayStatics::PlaySound2D(
		// 			this,                       // WorldContextObject
		// 			WeaponData->HitMarkerSound, // Sound
		// 			1.0f,                       // Volume Multiplier
		// 			1.0f,                       // Pitch Multiplier
		// 			0.0f,                       // Start Time
		// 			nullptr,                    // Concurrency Settings
		// 			nullptr,                    // Owning Actor
		// 			true                        // Is UI Sound
		// 		);
		// 	}
		// }
		bIsDead = IDamageable::Execute_IsDead(HitActor);
	}
    
	return bIsDead;
}
