// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LootItemRow.generated.h"

/**
 * DataTable Row for Loot Items
 * 엑셀/CSV에서 임포트 가능
 */
USTRUCT(BlueprintType)
struct FF_API FLootItemRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 아이템 PrimaryAssetId */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPrimaryAssetId ItemID;

	/** 아이템 최소 가치 (컨테이너 필터링용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinValue = 0;

	/** 아이템 최대 가치 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxValue = 100;

	/** 드롭 가중치 (높을수록 자주 드롭) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight = 1.f;

	/** 최소 드롭 수량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinQuantity = 1;

	/** 최대 드롭 수량 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxQuantity = 1;
};
