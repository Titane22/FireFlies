// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "MasterProjectile.generated.h"

class UProjectileMovementComponent;

UCLASS()
class FF_API AMasterProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMasterProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** 
	 * 발사 처리
	 * @param Direction 발사 방향 (정규화 되지 않아도 됨)
	 * @param Power     0.0~1.0 충전 비율 (0 = 최소 힘, 1 = 최대 힘)
	 */
	virtual void Fire(FVector Direction, float Power = 1.0f);
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* ProjectileCollision;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY()
	UProjectileMovementComponent* ProjectileMovement;

	/** 기본 발사 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float BaseSpeed = 1000.f;

	/** 최소 발사 속도 비율 (너무 살살 쏴도 약간은 나가도록) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float MinSpeedRatio = 0.3f;
};
