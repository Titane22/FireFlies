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
		// TODO: 루팅 오브젝트 별 애니메이션 삽입 및 루팅/인벤토리 해제 로직 개선
		Player->OpenLootingUI(InventorySystem);
	}

	return Super::ExecuteInteraction_Implementation(Context);
}
