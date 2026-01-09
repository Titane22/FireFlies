// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Data/EquipmentTypes.h"
#include "Gameplay/Data/ItemData.h"
#include "EquipmentSystem.generated.h"

class AFFCharacter;
class UInventorySystem;
class AMasterWeapon;
class UWeaponData;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Equip       UMETA(DisplayName = "Equip"),
	Unequip     UMETA(DisplayName = "Unequip")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentStateChangedDelegate);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FF_API UEquipmentSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UEquipmentSystem();

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void Equip(EEquipmentSlot Slot, UItemData* ItemData);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	UWeaponData* Unequip(EEquipmentSlot Slot);
	// 최적화된 새 API: 한 번의 호출로 무기 전환
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SwitchToWeapon(EEquipmentSlot TargetSlot);

	/**
	 * 새 무기 픽업 및 장착
	 * @param NewWeaponClass - 새로 장착할 무기 클래스
	 * @param TargetSlot - 장착할 슬롯 (Primary 또는 Handgun)
	 * @param OutDroppedWeaponClass - 드롭된 기존 무기 클래스 (출력)
	 * @return 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool PickupAndEquipWeapon(TSubclassOf<AMasterWeapon> NewWeaponClass, EEquipmentSlot TargetSlot, TSubclassOf<AMasterWeapon>& OutDroppedWeaponClass);

	// 레거시 API (하위 호환성을 위해 유지)
	UFUNCTION()
	void EquipWeapon(FName SocketName, EEquipmentSlot WeaponSlot);

	UFUNCTION()
	void UnequipWeapon(FName SocketName, EEquipmentSlot WeaponSlot);

	UFUNCTION()
	void EquipFromInventory(FGuid InstanceID, EEquipmentSlot TargetSlot);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool GetEquipmentSlot(EEquipmentSlot Slot, FEquipmentSlot& OutEquipSlot);

	UFUNCTION()
	void SetChildActorForSlot(EEquipmentSlot Slot, UChildActorComponent* ChildActor);

	UFUNCTION()
	UChildActorComponent* GetChildActorForSlot(EEquipmentSlot Slot);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool IsEquipped(EEquipmentSlot Slot);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY()
	AFFCharacter* CharacterRef;

	UPROPERTY()
	UInventorySystem* OwnerInventoryRef;
	
	// Blueprint에서 무기 클래스를 설정하세요 (Data-Driven)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AMasterWeapon> PrimaryWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AMasterWeapon> HandgunWeaponClass;

	// 현재 장착된 무기 슬롯 (None = 맨손)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	EEquipmentSlot CurrentEquippedSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Default")
	TMap<EEquipmentSlot, UWeaponData*> DefaultEquipments;

	TMap<EEquipmentSlot, UChildActorComponent*> SlotToChildActor;
	
	UPROPERTY(BlueprintReadOnly, Category = "Equipment")
	TMap<EEquipmentSlot, FEquipmentSlot> Equipped;

	UPROPERTY()
	FOnEquipmentStateChangedDelegate OnEquipmentStateChanged;
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AMasterWeapon> CurrentWeaponClass;
};
