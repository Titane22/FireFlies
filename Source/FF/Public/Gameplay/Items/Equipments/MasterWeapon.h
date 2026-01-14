// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Items/Equipments/EquipmentBase.h"
#include "MasterWeapon.generated.h"

class AFFCharacter;
class AInteraction;
class UWeaponData;
class UWeaponSystem;
class APlayer_Base;
class AIWeaponPickup;

UCLASS()
class FF_API AMasterWeapon : public AEquipmentBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMasterWeapon();

	virtual void Fire();
	virtual void Reload();
	
	// Hit 처리 함수
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool ApplyHit(const FHitResult HitResult, bool& ValidHit);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// 컴포넌트들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWeaponSystem* WeaponSystem;

	// Interaction Weapon Pickup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AInteraction> WeaponPickupClass;  

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UWeaponData* WeaponData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bReloading;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bAutoReload;

protected:
	USceneComponent* Muzzle;

private:
	// 컴포넌트 초기화 함수
	void InitializeComponents();

	// Fire helper functions
	void ApplyCameraShake(APlayerController* PC);
	bool PerformCameraTrace(APlayerCameraManager* CameraManager, FHitResult& OutHitResult);
	void ExecuteFireSequence(const FHitResult& CameraHitResult);

	void FireBullet(FHitResult Hit, bool bReturnHit);

	void FireFX();

	void FireBlankTracer();

	void RandPointInCircle(float Radius, float& PointX, float& PointY);

public:
	void SetOwningCharacter(AFFCharacter* ToSetCharacter);

	float GetMaxAmmo() const;

	float GetCurrentAmmo() const;
};
