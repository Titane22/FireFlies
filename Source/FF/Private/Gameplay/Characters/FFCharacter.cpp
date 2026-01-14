// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Characters/FFCharacter.h"
#include "Components/CapsuleComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gameplay/Components/HealthSystem.h"
#include "Gameplay/Components/Hurtbox.h"
#include "Gameplay/Items/EquipmentSystem.h"
#include "Gameplay/Items/InventorySystem.h"
#include "Gameplay/Items/Equipments/MasterWeapon.h"

// Sets default values
AFFCharacter::AFFCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Primary = CreateDefaultSubobject<USceneComponent>("Primary");
	Handgun = CreateDefaultSubobject<USceneComponent>("Handgun");
	PrimaryChild = CreateDefaultSubobject<UChildActorComponent>("PrimaryChild");
	HandgunChild = CreateDefaultSubobject<UChildActorComponent>("HandgunChild");
	EquipmentSystem = CreateDefaultSubobject<UEquipmentSystem>("EquipmentSystem");
	InventorySystem = CreateDefaultSubobject<UInventorySystem>("InventorySystem");
	HealthComponent = CreateDefaultSubobject<UHealthSystem>("HealthComponent");
	Hurtbox = CreateDefaultSubobject<UHurtbox>("Hurtbox");
	FlashlightChild = CreateDefaultSubobject<UChildActorComponent>("FlashlightChild");

	Primary->SetupAttachment(RootComponent);
	Handgun->SetupAttachment(RootComponent);
	PrimaryChild->SetupAttachment(Primary);
	HandgunChild->SetupAttachment(Handgun);
	FlashlightChild->SetupAttachment(RootComponent);
	EquippedChilds.Add(EEquipmentSlot::Primary, PrimaryChild);
	EquippedChilds.Add(EEquipmentSlot::Handgun, HandgunChild);
}

void AFFCharacter::BeginPlay()
{
	Super::BeginPlay();
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

