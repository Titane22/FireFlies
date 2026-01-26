// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletTrace.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;
class UNiagaraComponent;

/**
 * 총알 궤적 Actor - 충돌 또는 시간 경과 시 자동 소멸
 * 연발 시 여러 개가 동시에 존재할 수 있음
 */
UCLASS()
class FF_API ABulletTrace : public AActor
{
	GENERATED_BODY()

public:
	ABulletTrace();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	/** 충돌 감지용 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionComponent;

	/** 시각적 메시 (선택적) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* TracerMesh;

	/** 발사체 이동 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	/** 총알 궤적 이펙트 (선택적) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* TraceEffect;

	/** 총알 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet")
	float BulletSpeed = 30000.0f;

	/** 최대 생존 시간 (초) - 이 시간이 지나면 자동 소멸 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet")
	float MaxLifeTime = 3.0f;

	/** 충돌 시 즉시 소멸 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet")
	bool bDestroyOnHit = true;

	/** 기본 데미지 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet|Damage")
	float BaseDamage = 20.0f;

	/** Hit Reaction에 적용할 충격력 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet|Damage")
	float ImpactForce = 150.0f;

private:
	void ProcessHit(AActor* HitActor, const FHitResult& HitResult);
};
