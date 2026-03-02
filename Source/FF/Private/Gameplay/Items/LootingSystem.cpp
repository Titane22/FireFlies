// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Items/LootingSystem.h"
#include "Gameplay/Items/InventorySystem.h"
#include "Gameplay/Data/LootItemRow.h"
#include "Gameplay/Data/ItemData.h"
#include "Engine/DataTable.h"
#include "Engine/AssetManager.h"

ULootingSystem::ULootingSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULootingSystem::SetInventorySystem(UInventorySystem* IS)
{
	OwnerIS = IS;
}

void ULootingSystem::BeginPlay()
{
	Super::BeginPlay();
}

void ULootingSystem::GenerateLoot()
{
	if (!LootSettings.LootTable || !OwnerIS)
	{
		UE_LOG(LogTemp, Warning, TEXT("LootingSystem::GenerateLoot - LootTable or OwnerIS is null"));
		return;
	}

	// DataTable에서 유효한 항목 필터링
	TArray<const FLootItemRow*> ValidEntries;

	const TMap<FName, uint8*>& RowMap = LootSettings.LootTable->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const FLootItemRow* Row = reinterpret_cast<const FLootItemRow*>(Pair.Value);
		if (!Row)
			continue;

		// 컨테이너 가치 범위와 아이템 가치 범위가 겹치는지 확인
		if (Row->MaxValue >= LootSettings.ContainerMinValue &&
			Row->MinValue <= LootSettings.ContainerMaxValue)
		{
			ValidEntries.Add(Row);
		}
	}

	if (ValidEntries.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("LootingSystem::GenerateLoot - No valid entries found"));
		return;
	}

	// 드롭할 아이템 수 결정
	int32 DropCount = FMath::RandRange(
		LootSettings.MinDropCount,
		LootSettings.MaxDropCount
	);

	UE_LOG(LogTemp, Log, TEXT("LootingSystem::GenerateLoot - Dropping %d items from %d valid entries"),
		DropCount, ValidEntries.Num());

	// 아이템 드롭
	UAssetManager& AssetManager = UAssetManager::Get();
	float TotalWeight = 0.0f;
	for (int32 i = 0; i < DropCount; ++i)
	{
		const FLootItemRow* Selected = SelectByWeight(ValidEntries);
		if (!Selected || !Selected->ItemID.IsValid())
			continue;

		// PrimaryAssetId로 ItemData 로드
		UItemData* LoadedItemData = Cast<UItemData>(AssetManager.GetPrimaryAssetObject(Selected->ItemID));
		if (!LoadedItemData)
		{
			// 동기 로드 시도
			FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(Selected->ItemID);
			LoadedItemData = Cast<UItemData>(AssetPath.TryLoad());
		}

		if (!LoadedItemData)
		{
			UE_LOG(LogTemp, Warning, TEXT("LootingSystem::GenerateLoot - Failed to load ItemData: %s"),
				*Selected->ItemID.ToString());
			continue;
		}
		TotalWeight += Selected->Weight;
		LoadedItemData->bIsRevealed = false;

		// 수량 결정
		int32 Quantity = FMath::RandRange(Selected->MinQuantity, Selected->MaxQuantity);

		// 인벤토리에 추가
		OwnerIS->TryAddItemEmptySpot(LoadedItemData, Quantity);

		UE_LOG(LogTemp, Log, TEXT("LootingSystem::GenerateLoot - Added %s x%d"),
			*LoadedItemData->ItemName.ToString(), Quantity);
	}
	UE_LOG(LogTemp, Log, TEXT("LootingSystem::GenerateLoot - Total Weight %lf"),
			TotalWeight);
}

const FLootItemRow* ULootingSystem::SelectByWeight(const TArray<const FLootItemRow*>& Entries)
{
	if (Entries.IsEmpty())
		return nullptr;

	// 총 가중치 계산
	float TotalWeight = 0.0f;
	for (const FLootItemRow* Entry : Entries)
	{
		TotalWeight += Entry->Weight;
	}

	if (TotalWeight <= 0.0f)
		return Entries[0];

	// 랜덤 선택
	float Rand = FMath::RandRange(0.0f, TotalWeight);

	for (const FLootItemRow* Entry : Entries)
	{
		Rand -= Entry->Weight;
		if (Rand <= 0.0f)
			return Entry;
	}

	return Entries.Last();
}
