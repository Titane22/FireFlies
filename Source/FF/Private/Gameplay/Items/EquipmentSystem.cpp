// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/EquipmentSystem.h"
#include "Gameplay/Items/InventorySystem.h"
#include "Gameplay/Items/Equipments/WeaponSystem.h"
#include "Gameplay/Items/Equipments/MasterWeapon.h"
#include "Gameplay/Characters/FFCharacter.h"
#include "Gameplay/Data/EquipmentTypes.h"
#include "Gameplay/Data/WeaponData.h"
#include "Gameplay/Data/InventoryTypes.h"

// Sets default values for this component's properties
UEquipmentSystem::UEquipmentSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentEquippedSlot = EEquipmentSlot::None;
}

void UEquipmentSystem::BeginPlay()
{
	Super::BeginPlay();
	
	// 무기 클래스는 Blueprint에서 설정됩니다 (Data-Driven)
	for (const auto& Pair : DefaultEquipments)
	{
		EEquipmentSlot Slot = Pair.Key;
		UItemData* ItemData = Pair.Value;

		if (!ItemData)
		{
			UE_LOG(LogTemp, Error, TEXT("UEquipmentSystem::BeginPlay - ItemData is NULL"));
			continue;
		}
		FEquipmentSlot EquipmentSlot;
		EquipmentSlot.ItemData = ItemData;
		EquipmentSlot.EquipmentClass = ItemData->EquipmentClass;
		EquipmentSlot.Slot = Slot;
		
		Equipped.Add(Slot, EquipmentSlot);
		UE_LOG(LogTemp, Log, TEXT("[EquipmentSystem] Initialized slot %d with %s"),
			  (int32)Slot, *ItemData->ItemName.ToString());
	}
}

void UEquipmentSystem::Equip(EEquipmentSlot Slot, UItemData* ItemData, bool bShouldEquipToHand)
{
	if (!ItemData || !CharacterRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Equip] - Item Data or CharacterRef is NULL"));
		return;
	}
	FEquipmentSlot EquipSlot;
	EquipSlot.ItemData = ItemData;
	EquipSlot.EquipmentClass = ItemData->EquipmentClass;
	EquipSlot.Slot = Slot;
	
	// 1. 타겟 Child Actor 찾기
	UChildActorComponent** TargetChildPtr = CharacterRef->EquippedChilds.Find(Slot);
	if (!TargetChildPtr || !*TargetChildPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Equip] - Target Child is NULL for slot %d"), (int32)Slot);
		return;
	}

	UChildActorComponent* TargetChild = *TargetChildPtr;
	// 2. 타겟 Child Actor에 등록하기
	SetChildActorForSlot(Slot, TargetChild);
	if (!TargetChild->GetChildActor() && EquipSlot.EquipmentClass)
	{
		TargetChild->SetChildActorClass(EquipSlot.EquipmentClass);
		TargetChild->CreateChildActor();

		// Disable physics and interaction collision immediately after creation
		if (AMasterWeapon* Weapon = Cast<AMasterWeapon>(TargetChild->GetChildActor()))
		{
			if (Weapon->EquipmentMesh)
			{
				Weapon->EquipmentMesh->SetSimulatePhysics(false);
				Weapon->EquipmentMesh->SetEnableGravity(false);
			}
			// Disable interaction collision when equipped
			if (Weapon->InteractCollision)
			{
				Weapon->InteractCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
			UE_LOG(LogTemp, Log, TEXT("[Equip] Disabled physics and interaction for newly created weapon"));
		}
	}
	// 3. 타겟 Child Actor에 붙이기
	if (!CharacterRef->GetMesh())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Equip] - Character's Mesh is NULL"));
		return;
	}
	
	// Set WeaponSystem reference
	if (AMasterWeapon* Weapon = Cast<AMasterWeapon>(TargetChild->GetChildActor()))
	{
		if (Weapon->WeaponSystem)
		{
			Weapon->WeaponSystem->CharacterRef = CharacterRef;
		}
		// Note: Physics is automatically ignored when attached to character
	}

	FName SocketName;
	if (bShouldEquipToHand)
	{
		SocketName = ItemData->EquipSocketName;
	}
	else
	{
		SocketName = ItemData->UnequipSocketName;
	}
	// Attach to back (storage state)
	TargetChild->AttachToComponent(
		CharacterRef->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		SocketName
	);

	Equipped.Emplace(Slot, EquipSlot);
}

UWeaponData* UEquipmentSystem::Unequip(EEquipmentSlot Slot)
{
	FEquipmentSlot* EquipSlot = Equipped.Find(Slot);
	if (!EquipSlot || EquipSlot->ItemData.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Unequip] No equipment in slot %d"), (int32)Slot);
		return nullptr;
	}
	
	UWeaponData* WeaponData = Cast<UWeaponData>(EquipSlot->ItemData.Get());
	if (!WeaponData)
	{
		UE_LOG(LogTemp, Error, TEXT("[Unequip] ItemData is not WeaponData"));
		return nullptr;
	}

	UChildActorComponent* TargetChild = GetChildActorForSlot(Slot);
	if (TargetChild && TargetChild->GetChildActor())
	{
		AMasterWeapon* Weapon = Cast<AMasterWeapon>(TargetChild->GetChildActor());
		if (Weapon)
			Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		TargetChild->DestroyChildActor();
	}

	Equipped.Remove(Slot);
	if (CurrentEquippedSlot == Slot)
	{
		CurrentEquippedSlot = EEquipmentSlot::None;
	}
	
	return WeaponData;
}

void UEquipmentSystem::EquipWeapon(FName SocketName, EEquipmentSlot WeaponSlot)
{
	if (!CharacterRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("UEquipmentSystem::EquipWeapon CharacterRef is Null"));
		return;
	}

	// WeaponSlot에 따라 적절한 ChildActorComponent 선택
	UChildActorComponent* TargetChild = GetChildActorForSlot(WeaponSlot);

	if (!TargetChild)
	{
		UE_LOG(LogTemp, Warning, TEXT("UEquipmentSystem::EquipWeapon TargetChild is Null"));
		return;
	}

	// 무기가 이미 존재하는지 확인 (BeginPlay에서 생성됨)
	if (!TargetChild->GetChildActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("UEquipmentSystem::EquipWeapon ChildActor is Null"));
		return;
	}

	// 소켓에 부착 (무기를 재생성하지 않고 이동만)
	TargetChild->AttachToComponent(
		CharacterRef->GetMesh(),
		FAttachmentTransformRules(
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			true
		),
		SocketName
	);

	// CurrentEquippedSlot 업데이트
	CurrentEquippedSlot = WeaponSlot;
	UE_LOG(LogTemp, Log, TEXT("[EquipWeapon] CurrentEquippedSlot updated to: %d"), (int32)WeaponSlot);
}

void UEquipmentSystem::SwitchToWeapon(EEquipmentSlot TargetSlot)
{
	if (!CharacterRef)
	{
		UE_LOG(LogTemp, Error, TEXT("EquipmentSystem::SwitchToWeapon - CharacterRef is null"));
		return;
	}

	// 이미 같은 무기를 들고 있으면 무시
	if (CurrentEquippedSlot == TargetSlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("Already equipped %d"), (int32)TargetSlot);
		return;
	}

	// 1. 현재 장착된 무기를 등에 보관
	if (CurrentEquippedSlot != EEquipmentSlot::None)
	{
		FEquipmentSlot* EquipSlot = Equipped.Find(CurrentEquippedSlot);
		if (!EquipSlot || EquipSlot->ItemData.IsNull())
		{
			UE_LOG(LogTemp, Error, TEXT("[SwitchToWeapon] Current slot %d has no equipment"), (int32)CurrentEquippedSlot);
			return;
		}

		UItemData* ItemData = EquipSlot->ItemData.Get();
		if (!ItemData)
		{
			UE_LOG(LogTemp, Error, TEXT("[SwitchToWeapon] ItemData is null"));
			return;
		}

		UChildActorComponent* CurrentChild = GetChildActorForSlot(CurrentEquippedSlot);

		FName HolsterSocket = ItemData->UnequipSocketName;
		UnequipWeapon(HolsterSocket, CurrentEquippedSlot);
	}

	// 2. 새 무기를 손에 장착
	if (TargetSlot != EEquipmentSlot::None)
	{
		FEquipmentSlot* EquipSlot = Equipped.Find(TargetSlot);
		if (!EquipSlot || EquipSlot->ItemData.IsNull())
		{
			UE_LOG(LogTemp, Error, TEXT("[SwitchToWeapon] Target slot %d has no equipment"), (int32)TargetSlot);
			return;
		}

		UItemData* ItemData = EquipSlot->ItemData.Get();
		if (!ItemData)
		{
			UE_LOG(LogTemp, Error, TEXT("[SwitchToWeapon] ItemData is null"));
			return;
		}

		UChildActorComponent* TargetChild = GetChildActorForSlot(TargetSlot);

		FName HandSocket = ItemData->EquipSocketName;

		EquipWeapon(HandSocket, TargetSlot);
		if (TargetChild && TargetChild->GetChildActor())
		{
			TargetChild->AttachToComponent(
				CharacterRef->GetMesh(),
				FAttachmentTransformRules(
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					true
				),
				HandSocket
			);
		}
	}

	// 3. 상태 업데이트
	CurrentEquippedSlot = TargetSlot;
	//OnEquipmentStateChanged.Broadcast();
	UE_LOG(LogTemp, Log, TEXT("Switched to weapon slot: %d"), (int32)TargetSlot);
}

void UEquipmentSystem::UnequipWeapon(FName SocketName, EEquipmentSlot WeaponSlot)
{
	if (!CharacterRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("UEquipmentSystem::UnequipWeapon CharacterRef is Null"));
		return;
	}

	// WeaponSlot에 따라 적절한 ChildActorComponent 선택
	UChildActorComponent* TargetChild = GetChildActorForSlot(WeaponSlot);

	if (!TargetChild)
	{
		UE_LOG(LogTemp, Warning, TEXT("UEquipmentSystem::UnequipWeapon TargetChild is Null"));
		return;
	}

	// 무기가 이미 존재하는지 확인
	if (!TargetChild->GetChildActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("UEquipmentSystem::UnequipWeapon ChildActor is Null"));
		return;
	}

	// 소켓에 부착 (무기를 재생성하지 않고 이동만)
	TargetChild->AttachToComponent(
		CharacterRef->GetMesh(),
		FAttachmentTransformRules(
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget,
			true
		),
		SocketName
	);
	//OnEquipmentStateChanged.Broadcast();
}

void UEquipmentSystem::EquipFromInventory(FGuid InstanceID, EEquipmentSlot TargetSlot)
{
	if (!OwnerInventoryRef)
	{
		UE_LOG(LogTemp, Error, TEXT("[EquipFromInventory] OwnerInventoryRef is null"));
		return;
	}

	FItemSlot* ItemSlot = OwnerInventoryRef->FindItem(InstanceID);
	if (!ItemSlot)
	{
		UE_LOG(LogTemp, Error, TEXT("[EquipFromInventory] Item not found: %s"), *InstanceID.ToString());
		return;
	}

	UItemData* ItemData = ItemSlot->ItemData.Get();
	if (!ItemData)
	{
		UE_LOG(LogTemp, Error, TEXT("[EquipFromInventory] ItemData is null"));
		return;
	}

	FEquipmentSlot NewSlot;
	NewSlot.ItemData = ItemData;
	NewSlot.EquipmentClass = ItemData->EquipmentClass;

	// TODO: EquippedWeapons에 저장 및 실제 장착 로직
	FEquipmentSlot CurrentSlot;
	if (GetEquipmentSlot(TargetSlot, CurrentSlot))
	{
		Equipped[TargetSlot] = NewSlot;
	}
	else
	{
		Equipped.Add(TargetSlot, NewSlot);
	}
	
	OwnerInventoryRef->RemoveItem(InstanceID);
}

bool UEquipmentSystem::GetEquipmentSlot(EEquipmentSlot Slot, FEquipmentSlot& OutEquipSlot)
{
	FEquipmentSlot* EquipSlot = Equipped.Find(Slot);
	if (EquipSlot)
	{
		OutEquipSlot = *EquipSlot;
		return true;
	}
	
	return false;
}

void UEquipmentSystem::SetChildActorForSlot(EEquipmentSlot Slot, UChildActorComponent* ChildActor)
{
	SlotToChildActor.Add(Slot, ChildActor);
}

UChildActorComponent* UEquipmentSystem::GetChildActorForSlot(EEquipmentSlot Slot)
{
	UChildActorComponent** Found = SlotToChildActor.Find(Slot);
	return Found ? *Found : nullptr;
}

bool UEquipmentSystem::IsEquipped(EEquipmentSlot Slot)
{
	if (Equipped.Contains(Slot))
		return true;
	return false;
}

bool UEquipmentSystem::PickupAndEquipWeapon(TSubclassOf<AMasterWeapon> NewWeaponClass, EEquipmentSlot TargetSlot, TSubclassOf<AMasterWeapon>& OutDroppedWeaponClass)
{
	if (!CharacterRef || !NewWeaponClass || TargetSlot == EEquipmentSlot::None)
	{
		UE_LOG(LogTemp, Error, TEXT("PickupAndEquipWeapon: Invalid parameters"));
		return false;
	}

	// 대상 슬롯의 ChildActorComponent 가져오기
	UChildActorComponent* TargetChild = GetChildActorForSlot(TargetSlot);

	if (!TargetChild)
	{
		UE_LOG(LogTemp, Error, TEXT("PickupAndEquipWeapon: TargetChild is null"));
		return false;
	}

	// 기존 무기 클래스 저장 (드롭용)
	AMasterWeapon* CurrentWeapon = Cast<AMasterWeapon>(TargetChild->GetChildActor());
	if (CurrentWeapon)
	{
		OutDroppedWeaponClass = CurrentWeapon->GetClass();
	}

	// 반대 슬롯 무기를 홀스터로 이동 (CurrentEquippedSlot 상관없이, 반대 슬롯에 무기가 있으면 홀스터로)
	EEquipmentSlot OppositeSlot = (TargetSlot == EEquipmentSlot::Primary)
		? EEquipmentSlot::Handgun
		: EEquipmentSlot::Primary;

	UChildActorComponent* OppositeChild = GetChildActorForSlot(OppositeSlot);

	// 반대 슬롯에 무기가 있으면 홀스터로 이동
	if (OppositeChild && OppositeChild->GetChildActor())
	{
		FName HolsterSocket = (OppositeSlot == EEquipmentSlot::Primary)
			? FName("RifleHost_Socket")
			: FName("PistolHost_Socket");

		UE_LOG(LogTemp, Log, TEXT("PickupAndEquipWeapon: Moving opposite slot weapon to holster socket: %s"), *HolsterSocket.ToString());
		UnequipWeapon(HolsterSocket, OppositeSlot);
	}

	// 새 무기 클래스 설정
	TargetChild->SetChildActorClass(NewWeaponClass);

	// SetChildActorClass로 생성된 무기의 CharacterRef 수동 설정
	if (AMasterWeapon* NewWeapon = Cast<AMasterWeapon>(TargetChild->GetChildActor()))
	{
		if (NewWeapon->WeaponSystem)
		{
			NewWeapon->WeaponSystem->CharacterRef = CharacterRef;
			UE_LOG(LogTemp, Log, TEXT("PickupAndEquipWeapon: CharacterRef manually set for new weapon"));
		}
		// Note: Physics is automatically ignored when attached to character
	}

	// 무기 클래스 업데이트
	if (TargetSlot == EEquipmentSlot::Primary)
	{
		PrimaryWeaponClass = NewWeaponClass;
	}
	else
	{
		HandgunWeaponClass = NewWeaponClass;
	}

	// 손 소켓에 장착
	// TODO: 소켓도 WeaponData에 저장
	FName HandSocket = (TargetSlot == EEquipmentSlot::Primary)
		? FName("Rifle_Socket")
		: FName("Pistol_Socket");

	CurrentWeaponClass = NewWeaponClass;
	CurrentEquippedSlot = TargetSlot;

	EquipWeapon(HandSocket, TargetSlot);

	UE_LOG(LogTemp, Log, TEXT("PickupAndEquipWeapon: Equipped %s to slot %d"),
		*NewWeaponClass->GetName(), (int32)TargetSlot);

	return true;
}
