// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Gameplay/Data/ItemData.h"
#include "Gameplay/Data/InventoryTypes.h"
#include "Gameplay/Interfaces/Damageable.h"
#include "FFCharacter.generated.h"

class UInteractor;
class UHurtbox;
class UHealthSystem;
class UInventorySystem;
class UEquipmentSystem;
class UPhysicalAnimationComponent;
class UMeleeAttackSystem;
class AMasterWeapon;
struct FInputActionValue;

USTRUCT(BlueprintType)
struct FInputState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "State")
	bool bWantsToSprint;
	
	UPROPERTY(BlueprintReadWrite, Category = "State")
	bool bWantsToWalk;
	
	UPROPERTY(BlueprintReadWrite, Category = "State")
	bool bWantsToStrafe;
	
	UPROPERTY(BlueprintReadWrite, Category = "State")
	bool bWantsToAim;
};

UENUM(BlueprintType)
enum class EGait : uint8
{
	Walk	UMETA(DisplayName = "Walk"),
	Run		UMETA(DisplayName = "Run"),
	Sprint	UMETA(DisplayName = "Sprint")
};

UENUM(BlueprintType)
enum class ECameraStyle : uint8
{
	Far			UMETA(DisplayName = "Far"),
	Balanced	UMETA(DisplayName = "Balanced"),
	Close		UMETA(DisplayName = "Close"),
	FirstPerson	UMETA(DisplayName = "FirstPerson"),
};

UCLASS()
class FF_API AFFCharacter : public ACharacter, public IDamageable
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFFCharacter();
	
	UPhysicalAnimationComponent* GetPAC() const { return PAC; }
	UHurtbox* GetHurtbox() const { return Hurtbox; }

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void FlashOnOff();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Property Acess")
	bool IsRagdolling() const;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Property Acess")
	USkeletalMeshComponent* GetOverlaySkeletalMesh() const;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Property Acess")
	UStaticMeshComponent* GetOverlayStaticMesh() const;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Property Acess")
	float GetLandZVelocity() const;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Property Acess")
	bool IsJustLanded() const;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Overlay Pose")
	void SetOverlayPose(int32 PoseIndex);

	UFUNCTION(BlueprintCallable, Category = "OverlayPose")
	USkeletalMeshComponent* GetWeaponMesh() const;

	UFUNCTION(BlueprintCallable, Category = "OverlayPose")
	UStaticMeshComponent* GetWeaponStaticMesh() const;
	
	virtual float TakeDamage_Implementation(float DamageAmount, const FPointDamageEvent& DamageEvent, const FName HitBoneName,
					 AController* EventInstigator, AActor* DamageCauser) override;

	virtual bool IsDead_Implementation() const override;
	
	bool CanAttack();

	AMasterWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	UMeleeAttackSystem* GetMeleeAttackSystem() const;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* MeleeChildActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	UChildActorComponent* FlashlightChild;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ABP")
	FInputState InputState_CPP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ABP")
	EGait Gait_CPP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ABP")
	ECameraStyle CameraStyle_CPP;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Equipment|Action")
	bool bIsAiming;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Primary;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Secondary;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Handgun;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* MeleeScene;
	
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

	UPROPERTY(BlueprintReadWrite, Category = "Equipment")
	AMasterWeapon* CurrentWeapon;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bCanSwitchWeapon = true;

	bool IsCrouch;

	bool IsSprint;

	bool bIsDodging;

	//==============================================================================
	// Inventory-Weapon Mediation (조율)
	//==============================================================================

	/** 인벤토리 아이템 추가 시 호출 - 무기 자동 리로드 체크 */
	UFUNCTION()
	void OnInventoryItemAdded(const FItemSlot& AddedItem);

	/** 무기가 리로드 필요한지 확인 */
	bool DoesWeaponNeedReload(AMasterWeapon* Weapon) const;

	/** 인벤토리 델리게이트 바인딩 */
	void BindInventoryDelegates();
};
