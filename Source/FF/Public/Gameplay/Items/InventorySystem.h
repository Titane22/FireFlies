// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Data/InventoryTypes.h"
#include "InventorySystem.generated.h"

class UConsumableData;
// 아이템 추가/제거 시 브로드캐스트 (FItemSlot 전체 정보 전달)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemAdded, const FItemSlot&, AddedItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemRemoved, const FItemSlot&, RemovedItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConsumableUsed, UConsumableData*, ConsumableData, const FItemSlot&, RemovedItem);

class UMagazineData;

/**
 * Grid-based inventory system (Resident Evil style)
 * Items have different sizes and occupy multiple grid cells
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FF_API UInventorySystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventorySystem();

protected:
	virtual void BeginPlay() override;
	
	//==============================================================================
	// Helper Functions
	//==============================================================================

	/** Get 1D array index from 2D grid coordinates */
	int32 GetGridIndex(int32 Row, int32 Col) const;

	/** Check if grid coordinates are valid */
	bool IsValidGridPosition(int32 Row, int32 Col) const;

	/** Get the item at a specific grid cell (C++ only) */
	FItemSlot* GetItemAtCell(int32 Row, int32 Col);

	/** Update grid cells occupied by an item */
	void OccupyGridCells(const FItemSlot& Item);

	/** Clear grid cells occupied by an item */
	void ClearGridCells(const FItemSlot& Item);

	/** Recalculate total weight */
	void RecalculateWeight();
	
public:
	//==============================================================================
	// Public API
	//==============================================================================

	/** Check if an item can be placed at the specified position */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool CanPlaceItem(UItemData* ItemData, int32 GridRow, int32 GridCol) const;

	/** Add an item to the inventory at the specified position */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(UItemData* ItemData, int32 GridRow, int32 GridCol, int32 Quantity = 1);

	/** Add an item to the inventory at the specified position */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool TryAddItemEmptySpot(UItemData* ItemData, int32 Quantity = 1);
	
	/** Remove an item from the inventory by InstanceId */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(FGuid InstanceId);

	/** Move an existing item to a new grid position */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool MoveItem(FGuid InstanceId, int32 GridRow, int32 GridCol);
	
	/** Find an item by InstanceId (C++ only) */
	FItemSlot* FindItem(FGuid InstanceId);

	/** Find an item by InstanceId and return a copy (Blueprint-safe) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool FindItemCopy(FGuid InstanceId, FItemSlot& OutItem);

	/** Get all items in the inventory */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	const TArray<FItemSlot>& GetItems() const { return Items; }

	/** Get current/max weight */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetCurrentWeight() const { return CurrentWeight; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetMaxWeight() const { return MaxWeight; }

	/** Check if inventory is over weight limit */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsOverWeight() const { return CurrentWeight > MaxWeight; }

	/** Try to find an empty spot for an item (returns true if found, sets OutRow/OutCol) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool FindEmptySpot(UItemData* ItemData, int32& OutRow, int32& OutCol);

	/** Get the item at a specific grid cell and return a copy (Blueprint-safe) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool GetItemAtCellCopy(int32 Row, int32 Col, FItemSlot& OutItem);

	/** Initialize the grid (called automatically in BeginPlay, but can be called manually if needed) */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeGrid();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Ammo")
	bool HasAmmo(FGameplayTag RequiredAmmoTag);

	/** Get the magazine with the most ammo that matches RequiredAmmoTag (C++ only) */
	FItemSlot* GetBestMagazineSlot(FGameplayTag RequiredAmmoTag);

	/** 탄창을 인벤토리에 추가 (CurrentAmmo 포함) */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Ammo")
	bool AddMagazine(UMagazineData* MagazineData, int32 CurrentAmmo);

	//==============================================================================
	// Arrow System (Quantity-based ammo)
	//==============================================================================

	/** 인벤토리에 해당 태그의 화살이 있는지 확인 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Arrow")
	bool HasArrow(FGameplayTag RequiredArrowTag);

	/** 해당 태그의 화살 슬롯 중 가장 많은 수량을 가진 슬롯 반환 (C++ only) */
	FItemSlot* GetBestArrowSlot(FGameplayTag RequiredArrowTag);

	/** 화살 1개 소모 - 성공 시 소모된 ItemData 반환 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Arrow")
	UItemData* ConsumeArrow(FGameplayTag RequiredArrowTag);

	//==============================================================================
	// Consumable
	//==============================================================================
	UFUNCTION(BlueprintCallable, Category = "Inventory|Consumable")
	bool UseConsumable(FGuid InstanceID);
	
	//==============================================================================
	// Delegates
	//==============================================================================

	/** 아이템이 인벤토리에 추가되었을 때 */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemAdded OnItemAdded;

	/** 아이템이 인벤토리에서 제거되었을 때 */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemRemoved OnItemRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnConsumableUsed OnConsumableUsed;
	
protected:
	//==============================================================================
	// Grid Configuration
	//==============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Config")
	int32 RowCapacity = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Config")
	int32 ColCapacity = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Config")
	float MaxWeight = 100.0f;

	//==============================================================================
	// Inventory Data
	//==============================================================================

	/** All items in the inventory (placed and unplaced) */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Data")
	TArray<FItemSlot> Items;

	/** Grid state: each cell stores the InstanceId of the item occupying it */
	UPROPERTY()
	TArray<FGuid> GridCells;

	/** Current total weight of all items */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Data")
	float CurrentWeight = 0.0f;
	
public:
	
};
