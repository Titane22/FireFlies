// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemData.generated.h"

class AEquipmentBase;

/**
 * Equipment slot types
 */
UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
	None			UMETA(DisplayName = "None"),      // 맨손 상태
	Primary			UMETA(DisplayName = "Primary"),
	Secondary		UMETA(DisplayName = "Secondary"),
	Handgun			UMETA(DisplayName = "Handgun"),
	Melee			UMETA(DisplayName = "Melee")
};


UENUM(BlueprintType)
enum class EItemType : uint8
{
	None			UMETA(DisplayName = "None"),
	Equipment		UMETA(DisplayName = "Equipment"),
	Consumable		UMETA(DisplayName = "Consumable"),
	Ammo			UMETA(DisplayName = "Ammo")
};

/**
 * Item rarity levels
 */
UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	Common      UMETA(DisplayName = "Common"),
	Uncommon    UMETA(DisplayName = "Uncommon"),
	Rare        UMETA(DisplayName = "Rare"),
	Epic        UMETA(DisplayName = "Epic"),
	Legendary   UMETA(DisplayName = "Legendary")
};

/**
 * Base class for all item data in the game
 * Contains common properties shared by all items (weapons, consumables, equipment, etc.)
 */
UCLASS(Abstract)
class FF_API UItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UItemData();

	//==============================================================================
	// Basic Information
	//==============================================================================

	/** Display name of the item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Basic")
	FText ItemName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item|Basic")
	EItemType ItemType;
	
	/** Description of the item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Basic", meta = (MultiLine = true))
	FText ItemDescription;

	/** Icon displayed in inventory UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Basic")
	UTexture2D* ItemIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Details")
	TSubclassOf<AEquipmentBase> EquipmentClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	FName EquipSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	FName UnequipSocketName;
	//==============================================================================
	// Grid System (Resident Evil style inventory)
	//==============================================================================

	/** Width of the item in grid cells */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Grid", meta = (ClampMin = "1"))
	int32 GridWidth = 1;

	/** Height of the item in grid cells */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Grid", meta = (ClampMin = "1"))
	int32 GridHeight = 1;

	//==============================================================================
	// Stack System
	//==============================================================================

	/** Whether this item can be stacked */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Stack")
	bool bStackable;

	/** Maximum number of items in a single stack (only relevant if bStackable is true) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Stack", meta = (EditCondition = "bStackable", ClampMin = "1"))
	int32 MaxStackSize;

	//==============================================================================
	// Properties
	//==============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Equipment")
	EEquipmentSlot ValidSlot;
	
	/** Item rarity/quality level */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Properties")
	EItemRarity Rarity;

	/** Weight of a single item (used for inventory weight limits) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Properties", meta = (ClampMin = "0.0"))
	float Weight;

	//==============================================================================
	// Utility Functions
	//==============================================================================

	/** Get the total weight for a given quantity */
	UFUNCTION(BlueprintPure, Category = "Item")
	float GetTotalWeight(int32 Quantity) const;

	/** Check if the item is valid for use */
	UFUNCTION(BlueprintPure, Category = "Item")
	virtual bool IsValid() const;

	UFUNCTION(BlueprintPure, Category= "Item|Equipment")
	virtual bool CanEquipToSlot(EEquipmentSlot Slot) const;

	UFUNCTION(BlueprintPure, Category= "Item|Equipment")
	virtual bool IsEquippable() const;
};
