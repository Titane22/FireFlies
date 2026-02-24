#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Data/ItemData.h"
#include "InventoryTypes.generated.h"

/**
 * UI가 어떤 컨텍스트로 열렸는지 추적 — 컨텍스트별 Open/Close 애니메이션 매핑에 사용
 * 새 UI 타입 추가 시 여기에 값만 추가하면 됨
 */
UENUM(BlueprintType)
enum class EInventoryUIContext : uint8
{
	None      UMETA(DisplayName = "None"),
	Inventory UMETA(DisplayName = "Inventory"),
	Looting   UMETA(DisplayName = "Looting"),
};

/**
 * Represents a single item slot in the inventory
 * Holds reference to ItemData and instance-specific properties
 */
USTRUCT(BlueprintType)
struct FItemSlot
{
	GENERATED_BODY()

	//==============================================================================
	// Item Reference
	//==============================================================================

	/** Reference to the item data asset (WeaponData, ConsumableData, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSoftObjectPtr<UItemData> ItemData;

	//==============================================================================
	// Instance Properties
	//==============================================================================

	/** Grid position - Top-left row (-1 means not placed in grid) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Grid")
	int32 GridRow = -1;

	/** Grid position - Top-left column (-1 means not placed in grid) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Grid")
	int32 GridCol = -1;

	/** Number of items in this stack (must respect ItemData->MaxStackSize) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "0"))
	int32 Quantity = 1;

	/** Unique instance ID for this specific item (useful for trading, upgrading, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FGuid InstanceId;

	/** Optional durability for degradable items (0.0 = broken, 1.0 = pristine) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Durability = 1.0f;
	
	/** Current ammo count for magazines (-1 = not a magazine) */                                                                                                                                                           
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Magazine")                                                                                                                                                 
	int32 CurrentAmmo = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Magazine")
	int32 MaxAmmo = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Magazine")
	int32 ClipSize = -1;	
	//==============================================================================
	// Constructor & Operators
	//==============================================================================

	FItemSlot()
		: ItemData(nullptr)
		, GridRow(-1)
		, GridCol(-1)
		, Quantity(1)
		, InstanceId(FGuid::NewGuid())
		, Durability(1.0f)
		, CurrentAmmo(-1)
		, MaxAmmo(-1)
		, ClipSize(-1)
	{}

	FItemSlot(TSoftObjectPtr<UItemData> InItemData, int32 InQuantity = 1)
		: ItemData(InItemData)
		, GridRow(-1)
		, GridCol(-1)
		, Quantity(InQuantity)
		, InstanceId(FGuid::NewGuid())
		, Durability(1.0f)
		, CurrentAmmo(-1)
		, MaxAmmo(-1)
		, ClipSize(-1)
	{}

	bool operator==(const FItemSlot& Other) const
	{
		return InstanceId == Other.InstanceId;
	}

	bool operator!=(const FItemSlot& Other) const
	{
		return !(*this == Other);
	}

	//==============================================================================
	// Helper Functions
	//==============================================================================

	/** Check if this slot is valid (has valid ItemData) */
	bool IsValid() const
	{
		return !ItemData.IsNull();
	}

	/** Check if this slot is empty */
	bool IsEmpty() const
	{
		return ItemData.IsNull() || Quantity <= 0;
	}

	/** Get the loaded ItemData (loads if not already loaded) */
	UItemData* GetItemData() const
	{
		return ItemData.LoadSynchronous();
	}

	/** Get total weight of items in this slot */
	float GetTotalWeight() const
	{
		if (UItemData* Data = GetItemData())
		{
			return Data->GetTotalWeight(Quantity);
		}
		return 0.0f;
	}

	/** Check if this slot can stack with another slot */
	bool CanStackWith(const FItemSlot& Other) const
	{
		if (ItemData != Other.ItemData || ItemData.IsNull())
		{
			return false;
		}

		UItemData* Data = GetItemData();
		if (!Data || !Data->bStackable)
		{
			return false;
		}

		// Can stack if same item and total quantity doesn't exceed max stack size
		return (Quantity + Other.Quantity) <= Data->MaxStackSize;
	}

	/** Add quantity to this slot (returns overflow amount if exceeds max stack) */
	int32 AddQuantity(int32 Amount)
	{
		if (Amount <= 0 || ItemData.IsNull())
		{
			return Amount;
		}

		UItemData* Data = GetItemData();
		if (!Data)
		{
			return Amount;
		}

		int32 MaxAdd = Data->bStackable ? (Data->MaxStackSize - Quantity) : 0;
		int32 ActualAdd = FMath::Min(Amount, MaxAdd);

		Quantity += ActualAdd;
		return Amount - ActualAdd; // Return overflow
	}

	/** Remove quantity from this slot (returns actual amount removed) */
	int32 RemoveQuantity(int32 Amount)
	{
		int32 ActualRemove = FMath::Min(Amount, Quantity);
		Quantity -= ActualRemove;
		return ActualRemove;
	}

	/** Clear this slot */
	void Clear()
	{
		ItemData = nullptr;
		GridRow = -1;
		GridCol = -1;
		Quantity = 0;
		Durability = 1.0f;
		CurrentAmmo = -1;
		MaxAmmo = -1;
		ClipSize = -1;
	}

	//==============================================================================
	// Grid Helper Functions
	//==============================================================================

	/** Check if this item is placed in the grid */
	bool IsPlacedInGrid() const
	{
		return GridRow >= 0 && GridCol >= 0;
	}

	/** Check if this item occupies a specific grid cell */
	bool OccupiesCell(int32 Row, int32 Col) const
	{
		if (!IsPlacedInGrid())
		{
			return false;
		}

		UItemData* Data = GetItemData();
		if (!Data)
		{
			return false;
		}

		// Check if (Row, Col) is within the item's bounding box
		return Row >= GridRow && Row < (GridRow + Data->GridHeight) &&
		       Col >= GridCol && Col < (GridCol + Data->GridWidth);
	}

	/** Get all grid cells occupied by this item */
	TArray<FIntPoint> GetOccupiedCells() const
	{
		TArray<FIntPoint> Cells;

		if (!IsPlacedInGrid())
		{
			return Cells;
		}

		UItemData* Data = GetItemData();
		if (!Data)
		{
			return Cells;
		}

		for (int32 Row = GridRow; Row < GridRow + Data->GridHeight; ++Row)
		{
			for (int32 Col = GridCol; Col < GridCol + Data->GridWidth; ++Col)
			{
				Cells.Add(FIntPoint(Col, Row));
			}
		}

		return Cells;
	}

	// 탄창인지 확인
	bool IsMagazine() const
	{
		return CurrentAmmo >= 0;
	}

	// 탄창으로 초기화
	void InitializeAsMagazine(int32 AmmoCount, int32 BaseClipSize, int32 TotalAmmoCount)
	{
		CurrentAmmo = AmmoCount;
		ClipSize = BaseClipSize;
		MaxAmmo = TotalAmmoCount;
	}

	// 탄약 소모, 실제 소모량 반환
	int32 ConsumeAmmo(int32 Amount = 1)
	{
		if (CurrentAmmo < 0)
			return 0;
		int32 Consumed = FMath::Min(Amount, CurrentAmmo);
		CurrentAmmo -= Consumed;
		return Consumed;
	}

	// 탄약 추가, 오버플로우 반환
	int32 AddAmmo(int32 Amount, int32 MaxCapacity)
	{
		if(CurrentAmmo < 0)
			CurrentAmmo = 0;
		int32 AvailableSpace = MaxCapacity - CurrentAmmo;
		int32 ActualAdd =  FMath::Min(Amount, AvailableSpace);
		CurrentAmmo += ActualAdd;
		return Amount - ActualAdd;
	}
};