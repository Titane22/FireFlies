// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Items/Equipments/BulletTrace.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "Gameplay/Interfaces/Damageable.h"
#include "Gameplay/Items/Equipments/MasterWeapon.h"
#include "Gameplay/Data/WeaponData.h"
#include "Presentations/HUD/PlayerHUD.h"
#include "Kismet/GameplayStatics.h"

ABulletTrace::ABulletTrace()
{
	PrimaryActorTick.bCanEverTick = false;

	// 충돌 컴포넌트 설정
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(5.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
	RootComponent = CollisionComponent;

	// 메시 컴포넌트 (선택적 - Blueprint에서 설정 가능)
	TracerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TracerMesh"));
	TracerMesh->SetupAttachment(RootComponent);
	TracerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Niagara 이펙트 컴포넌트 (선택적)
	TraceEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TraceEffect"));
	TraceEffect->SetupAttachment(RootComponent);

	// 발사체 이동 컴포넌트
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = BulletSpeed;
	ProjectileMovement->MaxSpeed = BulletSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;  // 총알은 중력 영향 없음

	// 기본 생존 시간 설정
	InitialLifeSpan = MaxLifeTime;
}

void ABulletTrace::BeginPlay()
{
	Super::BeginPlay();

	// 생존 시간 설정 (런타임에 변경된 경우 대비)
	SetLifeSpan(MaxLifeTime);

	// 발사체 속도 업데이트
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = BulletSpeed;
		ProjectileMovement->MaxSpeed = BulletSpeed;
	}

	// 충돌 이벤트 바인딩
	if (CollisionComponent)
	{
		CollisionComponent->OnComponentHit.AddDynamic(this, &ABulletTrace::OnHit);
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ABulletTrace::OnBeginOverlap);
	}
}

void ABulletTrace::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 자기 자신이나 Owner와 충돌은 무시
	if (OtherActor == this || OtherActor == GetOwner() || OtherActor == GetInstigator())
	{
		return;
	}

	ProcessHit(OtherActor, Hit);

	if (bDestroyOnHit)
	{
		Destroy();
	}
}

void ABulletTrace::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 자기 자신이나 Owner와 오버랩은 무시
	if (OtherActor == this || OtherActor == GetOwner() || OtherActor == GetInstigator())
	{
		return;
	}

	ProcessHit(OtherActor, SweepResult);

	if (bDestroyOnHit)
	{
		Destroy();
	}
}

void ABulletTrace::ProcessHit(AActor* HitActor, const FHitResult& HitResult)
{
	if (!HitActor || !HitActor->Implements<UDamageable>())
		return;

	// 물리 오브젝트에 충격 적용
	UPrimitiveComponent* HitComponent = HitResult.GetComponent();
	if (HitComponent && HitComponent->IsSimulatingPhysics())
	{
		FVector ImpulseDir = GetVelocity().GetSafeNormal();
		HitComponent->AddImpulseAtLocation(ImpulseDir * ImpactForce, HitResult.ImpactPoint);
	}

	// 거리별 데미지 감소 적용
	float FinalDamage = BaseDamage;
	AMasterWeapon* OwnerWeapon = Cast<AMasterWeapon>(GetOwner());
	APawn* InstigatorPawn = GetInstigator();
	if (OwnerWeapon && OwnerWeapon->WeaponData)
	{
		float Distance = InstigatorPawn
			? FVector::Dist(InstigatorPawn->GetActorLocation(), HitResult.ImpactPoint)
			: 0.f;
		FinalDamage = OwnerWeapon->WeaponData->GetDamageAtDistance(BaseDamage, Distance);
	}

	// 데미지 적용
	FVector ShotDirection = GetVelocity().GetSafeNormal();
	FPointDamageEvent DamageEvent(
		FinalDamage,
		HitResult,
		ShotDirection,
		nullptr
	);

	AController* InstigatorCtrl = InstigatorPawn ? InstigatorPawn->GetController() : nullptr;

	IDamageable::Execute_TakeDamage(
		HitActor,
		FinalDamage,
		DamageEvent,
		HitResult.BoneName,
		InstigatorCtrl,
		GetOwner()
	);

	// Projectile 개별 히트 피드백
	bool bIsDead = IDamageable::Execute_IsDead(HitActor);
	UWeaponData* WeaponData = OwnerWeapon ? OwnerWeapon->WeaponData : nullptr;
	if (WeaponData)
	{
		USoundBase* Sound = bIsDead ? WeaponData->KillSound : WeaponData->HitMarkerSound;
		if (Sound)
			UGameplayStatics::PlaySound2D(this, Sound);
	}

	// HUD 히트 마커 표시
	if (InstigatorPawn)
	{
		APlayerController* PC = Cast<APlayerController>(InstigatorPawn->GetController());
		if (PC)
		{
			APlayerHUD* HUD = Cast<APlayerHUD>(PC->GetHUD());
			if (HUD)
				HUD->ShowHitMarker(bIsDead);
		}
	}
}
