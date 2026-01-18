// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Items/LootContainer.h"
#include "Gameplay/Characters/Player_Base.h"
#include "Gameplay/Items/InventorySystem.h"
#include "Gameplay/Items/LootingSystem.h"

ALootContainer::ALootContainer()
{

	InventorySystem = CreateDefaultSubobject<UInventorySystem>("InventorySystem");
	LootingSystem = CreateDefaultSubobject<ULootingSystem>("LootingSystem");
}

void ALootContainer::BeginPlay()
{
	Super::BeginPlay();

	if (LootingSystem && InventorySystem)
	{
		LootingSystem->SetInventorySystem(InventorySystem);
		LootingSystem->SetLootActive(true);
	}
}

FInteractionResult ALootContainer::ExecuteInteraction_Implementation(const FInteractionContext& Context)
{
	// 첫 오픈 시 루트 생성
	if (bGenerateLootOnFirstOpen && !bHasGeneratedLoot)
	{
		if (LootingSystem && LootingSystem->CanLooting())
		{
			LootingSystem->GenerateLoot();
			bHasGeneratedLoot = true;
		}
	}

	// 루팅 UI 열기
	if (APlayer_Base* Player = Cast<APlayer_Base>(Context.InstigatorPawn))
	{
		// Player->OpenLootingUI(InventorySystem);
		Player->OpenLootingUI(InventorySystem);
	}

	// 부모 클래스의 이벤트 브로드캐스트 호출
	Super::ExecuteInteraction_Implementation(Context);

	return FInteractionResult::Success();
}
