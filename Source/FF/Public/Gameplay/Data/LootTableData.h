// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LootTableData.generated.h"

class UItemData;

USTRUCT(BlueprintType)
struct FLootItemEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UItemData* ItemData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MinValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxValue = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Weight = 1.f;
	
};

/**
 * 
 */
UCLASS(BlueprintType)
class FF_API ULootTableData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FLootItemEntry> LootItems;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ContainerMinValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ContainerMaxValue = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MinDropCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxDropCount = 5;
};
