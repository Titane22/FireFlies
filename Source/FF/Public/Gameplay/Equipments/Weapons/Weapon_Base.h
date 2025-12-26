// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Library/AnimationState.h"
#include "Weapon_Base.generated.h"

class UWeaponSystem;
class UWeaponData;
class AFFCharacter;

UCLASS()
class FF_API AWeapon_Base : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon_Base();

	virtual void Fire();
	virtual void Reload();

	UWeaponSystem* GetWeaponSystem() const { return WeaponSystem; }
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWeaponSystem* WeaponSystem;
	
	EAnimationState AnimationState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UWeaponData* WeaponData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bReloading;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bAutoReload;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|VFX")
	USoundBase* HitMarkerSound;
	
protected:
	UFUNCTION()
	bool ApplyHit(const FHitResult HitResult, bool& bValidHit);

};
