// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/Projectiles/MasterProjectile.h"

#include "DataWrappers/ChaosVDJointDataWrappers.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AMasterProjectile::AMasterProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProjectileCollision = CreateDefaultSubobject<UBoxComponent>(FName("ProjectileCollision"));
	RootComponent = ProjectileCollision;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AMasterProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMasterProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMasterProjectile::Fire(FVector Direction, float Power)
{
	if (ProjectileMovement)
		return;

	// 충전 비율 클램프
	const float ClampedPower = FMath::Clamp(Power, 0.f, 1.f);
	// 너무 약하게 쐈을 때도 조금은 나가도록 최소 비율 보장
	const float SpeedRatio = FMath::Lerp(MinSpeedRatio, 1.0f, ClampedPower);
	const float FinalSpeed = BaseSpeed * SpeedRatio;

	ProjectileMovement = NewObject<UProjectileMovementComponent>(this);
	ProjectileMovement->RegisterComponent();
	ProjectileMovement->SetUpdatedComponent(RootComponent);

	ProjectileMovement->InitialSpeed = FinalSpeed;
	ProjectileMovement->MaxSpeed = FinalSpeed;
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->Velocity = Direction.GetSafeNormal() * FinalSpeed;
	
	ProjectileMovement->Activate();
}

