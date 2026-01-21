// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Data/ConsumableData.h"

UConsumableData::UConsumableData()
{
	ItemName = FText::FromString("Consumable");
	bStackable = true;
	MaxStackSize = 99;

	ItemType = EItemType::Consumable;
}

FPrimaryAssetId UConsumableData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("Consumable_C", GetFName());
}
