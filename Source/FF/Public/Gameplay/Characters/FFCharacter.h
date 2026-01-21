// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Gameplay/Data/ItemData.h"
#include "Gameplay/Data/InventoryTypes.h"
#include "FFCharacter.generated.h"

class UInteractor;
class UHurtbox;
class UHealthSystem;
class UInventorySystem;
class UEquipmentSystem;
class UPhysicalAnimationComponent;
class AMasterWeapon;
struct FInputActionValue;

UCLASS()
class FF_API AFFCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFFCharacter();
	
	UPhysicalAnimationComponent* GetPAC() const { return PAC; }

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void FlashOnOff();

protected:
	virtual void BeginPlay();

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void SwitchWeapon(EEquipmentSlot Slot);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void SwitchToPrimaryWeapon();

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void SwitchToHandgunWeapon();
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TMap<EEquipmentSlot, UChildActorComponent*> EquippedChilds;

	// Weapon Child Actor Components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* PrimaryChild;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* SecondaryChild;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* HandgunChild;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	UChildActorComponent* FlashlightChild;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Primary;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Secondary;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Handgun;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UPhysicalAnimationComponent* PAC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UEquipmentSystem* EquipmentSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UInventorySystem* InventorySystem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UHealthSystem* HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UHurtbox* Hurtbox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UInteractor* Interactor;
	
	bool bCanSwitchWeapon = true;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bFiring;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Equipment|Action")
	bool bIsAiming;

	bool bCanFire = true;

	bool IsCrouch;

	bool IsSprint;

	bool bIsDodging;

	//==============================================================================
	// Inventory-Weapon Mediation (조율)
	//==============================================================================

	/** 인벤토리 아이템 추가 시 호출 - 무기 자동 리로드 체크 */
	UFUNCTION()
	void OnInventoryItemAdded(const FItemSlot& AddedItem);

	/** 현재 장착된 무기 반환 */
	AMasterWeapon* GetCurrentWeapon() const;

	/** 무기가 리로드 필요한지 확인 */
	bool DoesWeaponNeedReload(AMasterWeapon* Weapon) const;

	/** 인벤토리 델리게이트 바인딩 */
	void BindInventoryDelegates();
};
