// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/LootingSystem.h"
#include "Gameplay/Items/InventorySystem.h"
#include "Gameplay/Data/LootTableData.h"

// Sets default values for this component's properties
ULootingSystem::ULootingSystem()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void ULootingSystem::SetInventorySystem(UInventorySystem* IS)
{
	OwnerIS = IS;
}

// Called when the game starts
void ULootingSystem::BeginPlay()
{
	Super::BeginPlay();

}

void ULootingSystem::GenerateLoot()
{
	if (!LootTable || !OwnerIS)
	{
		UE_LOG(LogTemp, Error, TEXT("No Loot Table"));
		return;
	}

	TArray<const FLootItemEntry*> ValidEntries;
	for (const FLootItemEntry& Entry : LootTable->LootItems)
	{
		// 컨테이너 가치 범위와 아이템 가치 범위가 겹치는가?
		if (Entry.MaxValue >= LootTable->ContainerMinValue &&
			Entry.MinValue <= LootTable->ContainerMaxValue)
		{
			ValidEntries.Add(&Entry);
			UE_LOG(LogTemp, Warning, TEXT("Entry: %s"), *Entry.ItemData->ItemName.ToString());
		}
	}

	if (ValidEntries.IsEmpty())
		return;

	int32 DropCount = FMath::RandRange(
		LootTable->MinDropCount,
		LootTable->MaxDropCount
	);

	for (int32 i = 0; i < DropCount; ++i)
	{
		const FLootItemEntry* Selected = SelectByWeight(ValidEntries);
		if (!Selected || !Selected->ItemData)
			continue;

		// TODO: Cacluate Item Quantity
		OwnerIS->TryAddItemEmptySpot(Selected->ItemData);
		UE_LOG(LogTemp, Error, TEXT("Selected->ItemData: %s"), *Selected->ItemData->ItemName.ToString());
	}
}

const FLootItemEntry* ULootingSystem::SelectByWeight(TArray<const FLootItemEntry*> Entries)
{
	float TotalWeight = 0.0f;
	for (const auto* Entry : Entries)
	{
		TotalWeight += Entry->Weight;
	}

	float Rand = FMath::RandRange(0.0f, TotalWeight);

	for (const auto* Entry : Entries)
	{
		Rand -= Entry->Weight;
		if (Rand <= 0.f)
			return Entry;
	}
	return nullptr;
}
