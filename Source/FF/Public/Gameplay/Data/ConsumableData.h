// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Data/ItemData.h"
#include "ConsumableData.generated.h"

UENUM(BlueprintType)
enum class EConsumableEffect : uint8
{
	Hunger,
	Heal,
	Thirst
};
/**
 * 
 */
UCLASS()
class FF_API UConsumableData : public UItemData
{
	GENERATED_BODY()

public:
	UConsumableData();
	/** AssetManager용 PrimaryAssetId 반환 */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable")
	EConsumableEffect EffectType = EConsumableEffect::Hunger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable")
	float EffectValue = 0.f;
};
