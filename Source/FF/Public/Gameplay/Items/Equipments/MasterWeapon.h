// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Gameplay/Items/Equipments/EquipmentBase.h"
#include "Gameplay/Interfaces/Interactable.h"
#include "MasterWeapon.generated.h"

class AFFCharacter;
class AInteraction;
class UWeaponData;
class UWeaponSystem;
class APlayer_Base;
class AIWeaponPickup;
class UInteractionData;

UCLASS()
class FF_API AMasterWeapon : public AEquipmentBase, public IInteractable
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

	/** Spawn a dropped weapon in the world with physics enabled */
	static AMasterWeapon* SpawnDroppedWeapon(UWorld* World, UWeaponData* WeaponData, const FVector& Location, const FRotator& Rotation, AActor* Owner = nullptr);

	//==============================================================================
	// IInteractable Interface
	//==============================================================================

	virtual FInteractionResult ExecuteInteraction_Implementation(const FInteractionContext& Context) override;
	virtual bool CanInteract_Implementation(AController* InstigatorRef) const override;
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual bool IsHoldInteraction_Implementation() const override;
	virtual float GetHoldDuration_Implementation() const override;
	virtual bool IsSingleUse_Implementation() const override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;
	virtual void OnInteractionStarted_Implementation(const FInteractionContext& Context) override;
	virtual void OnInteractionCancelled_Implementation(const FInteractionContext& Context) override;
	virtual AActor* GetInteractableActor_Implementation() override;
	virtual EInteractiveType GetInteractionType_Implementation() const override;

protected:
	/** 하이라이트 상태 */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bIsHighlighted = false;
};
