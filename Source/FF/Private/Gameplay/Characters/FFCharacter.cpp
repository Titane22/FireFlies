// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Characters/FFCharacter.h"
#include "Components/CapsuleComponent.h"
#include "InputActionValue.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gameplay/Components/HealthSystem.h"
#include "Gameplay/Components/Hurtbox.h"
#include "Gameplay/Items/EquipmentSystem.h"
#include "Gameplay/Items/InventorySystem.h"
#include "Gameplay/Items/Equipments/MasterWeapon.h"
#include "Gameplay/Items/Equipments/MasterMagazine.h"
#include "Gameplay/Items/Equipments/WeaponAttackSystem.h"
#include "Gameplay/Items/Interaction/Interactor.h"
#include "Gameplay/Data/MagazineData.h"
#include "Gameplay/Data/WeaponData.h"
#include "Perception/AISense_Damage.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"

// Sets default values
AFFCharacter::AFFCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Primary = CreateDefaultSubobject<USceneComponent>("Primary");
	Secondary = CreateDefaultSubobject<USceneComponent>("Secondary");
	Handgun = CreateDefaultSubobject<USceneComponent>("Handgun");
	MeleeScene = CreateDefaultSubobject<USceneComponent>("Melee Scene");
	PrimaryChild = CreateDefaultSubobject<UChildActorComponent>("PrimaryChild");
	SecondaryChild = CreateDefaultSubobject<UChildActorComponent>("SecondaryChild");
	HandgunChild = CreateDefaultSubobject<UChildActorComponent>("HandgunChild");
	MeleeChildActor = CreateDefaultSubobject<UChildActorComponent>("Melee Child Actor");
	
	EquipmentSystem = CreateDefaultSubobject<UEquipmentSystem>("EquipmentSystem");
	InventorySystem = CreateDefaultSubobject<UInventorySystem>("InventorySystem");
	HealthComponent = CreateDefaultSubobject<UHealthSystem>("HealthComponent");
	Hurtbox = CreateDefaultSubobject<UHurtbox>("Hurtbox");
	Interactor = CreateDefaultSubobject<UInteractor>("Interactor");
	FlashlightChild = CreateDefaultSubobject<UChildActorComponent>("FlashlightChild");
	PAC = CreateDefaultSubobject<UPhysicalAnimationComponent>("PhysicalAnimationComponent");
	
	Primary->SetupAttachment(RootComponent);
	Secondary->SetupAttachment(RootComponent);
	Handgun->SetupAttachment(RootComponent);
	MeleeScene->SetupAttachment(RootComponent);
	PrimaryChild->SetupAttachment(Primary);
	SecondaryChild->SetupAttachment(Secondary);
	HandgunChild->SetupAttachment(Handgun);
	MeleeChildActor->SetupAttachment(MeleeScene);
	FlashlightChild->SetupAttachment(RootComponent);

	SecondaryChild->RegisterComponentWithWorld(GetWorld());
}

USkeletalMeshComponent* AFFCharacter::GetWeaponMesh() const
{
	if (!CurrentWeapon)
		return nullptr;

	return CurrentWeapon->EquipmentMesh;
}

UStaticMeshComponent* AFFCharacter::GetWeaponStaticMesh() const
{
	if (!CurrentWeapon)
		return nullptr;
	
	return CurrentWeapon->EquipmentStaticMesh;
}

float AFFCharacter::TakeDamage_Implementation(float DamageAmount, const FPointDamageEvent& DamageEvent, const FName HitBoneName,
                                              AController* EventInstigator, AActor* DamageCauser)
{
	if (!HealthComponent || !Hurtbox)
		return 0.f;
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, TEXT("HitActor2222222"));

	FVector HitLocation = DamageEvent.HitInfo.ImpactPoint;
	FVector HitDirection = DamageEvent.ShotDirection;
	
	// TODO: Procedural Hit Reaction 
	//Hurtbox->TriggerHitReaction(HitLocation, HitDirection, HitBoneName, 1000.f);
	
	//float Multiplier = Hurtbox->GetDamageMultiplier(HitBoneName);
	float ModifiedDamage = DamageAmount;
	
	HealthComponent->ApplyDamage(ModifiedDamage);
	if (EventInstigator)
	{
		UAISense_Damage::ReportDamageEvent(
			GetWorld(),
			this,                              
			EventInstigator,   
			ModifiedDamage,
			GetActorLocation(),                
			GetActorLocation()                 
		);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange,
			TEXT("Damage Event NOT Reported: EventInstigator or Character is NULL"));
	}
	
	return ModifiedDamage;
}

bool AFFCharacter::IsDead_Implementation() const
{
	return HealthComponent && HealthComponent->IsDead();
}

void AFFCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	EquippedChilds.Add(EEquipmentSlot::Primary, PrimaryChild);
	EquippedChilds.Add(EEquipmentSlot::Secondary, SecondaryChild);
	EquippedChilds.Add(EEquipmentSlot::Handgun, HandgunChild);
	EquippedChilds.Add(EEquipmentSlot::Melee, MeleeChildActor);
	
	// 미등록 ChildActorComponent 강제 등록
	for (const auto& Pair : EquippedChilds)
	{
		UChildActorComponent* Child = Pair.Value;
		if (Child && !Child->IsRegistered())
		{
			Child->RegisterComponent();
			UE_LOG(LogTemp, Log, TEXT("[FFCharacter] Manually registered %s (Success: %s)"),
				*Child->GetName(),
				Child->IsRegistered() ? TEXT("Yes") : TEXT("No"));
		}
	}

	if (EquipmentSystem)
	{
		EquipmentSystem->CharacterRef = this;
	}
	// TODO: if (InventorySystem)
	// {
	// 	InventorySystem->CharacterRef = this;
	// }

	if (HealthComponent)
	{
		HealthComponent->CharacterRef = this;
	}
	if (Hurtbox)
	{
		Hurtbox->CharacterRef = this;
	}
	if (Interactor)
	{
		//Interactor->Chac
	}
	
	if (EquipmentSystem)
	{
		for (auto& DefaultEquip : EquipmentSystem->DefaultEquipments)
		{
			EEquipmentSlot Slot = DefaultEquip.Key;
			UItemData* ItemData = DefaultEquip.Value;
			
			if (ItemData)
			{
				EquipmentSystem->Equip(Slot, ItemData);
			}
		}
	}

	// 인벤토리-무기 조율을 위한 델리게이트 바인딩
	BindInventoryDelegates();
}

void AFFCharacter::SwitchWeapon(EEquipmentSlot Slot)
{
}

void AFFCharacter::SwitchToPrimaryWeapon()
{
}

void AFFCharacter::SwitchToHandgunWeapon()
{
}

void AFFCharacter::FlashOnOff()
{
	if (FlashlightChild->IsVisible())
	{
		FlashlightChild->SetVisibility(false);
	}
	else
	{
		FlashlightChild->SetVisibility(true);
	}
}

//==============================================================================
// Inventory-Weapon Mediation
//==============================================================================

void AFFCharacter::BindInventoryDelegates()
{
	if (InventorySystem)
	{
		InventorySystem->OnItemAdded.AddDynamic(this, &AFFCharacter::OnInventoryItemAdded);
		UE_LOG(LogTemp, Log, TEXT("FFCharacter: Bound to InventorySystem delegates"));
	}
}

void AFFCharacter::OnInventoryItemAdded(const FItemSlot& AddedItem)
{
	// 현재 무기 확인
	if (!CurrentWeapon)
		return;

	// 무기가 리로드 필요한지 확인
	if (!DoesWeaponNeedReload(CurrentWeapon))
		return;

	// 추가된 아이템이 탄창인지 확인
	UMagazineData* MagData = Cast<UMagazineData>(AddedItem.ItemData.Get());
	if (!MagData)
		return;

	// 무기와 호환되는 탄창인지 확인
	if (!CurrentWeapon->WeaponData)
		return;

	if (!MagData->IsCompatibleWith(CurrentWeapon->WeaponData->RequiredAmmoType))
		return;

	// 탄약이 있는 탄창인지 확인
	if (AddedItem.CurrentAmmo <= 0)
		return;

	
	// 자동 리로드 설정 확인
	if (CurrentWeapon->AttackSystem && CurrentWeapon->AttackSystem->bAutoReload)
	{
		UE_LOG(LogTemp, Log, TEXT("FFCharacter: Auto-reload triggered by new magazine"));
		CurrentWeapon->Reload();
	}
}

bool AFFCharacter::DoesWeaponNeedReload(AMasterWeapon* Weapon) const
{
	if (!Weapon)
		return false;

	// 탄창이 없거나 비어있으면 리로드 필요
	if (!Weapon->CurrentMagazine)
		return true;

	return !Weapon->CurrentMagazine->HasAmmo();
}

bool AFFCharacter::CanAttack()
{
	bool bCanJumpNow = CanJump();  // 한 번만 호출하고 저장
	bool bCanShoot = !IsSprint && !bIsDodging && bCanJumpNow && bCanSwitchWeapon && bCanAttack;
	
	return bCanShoot;
}