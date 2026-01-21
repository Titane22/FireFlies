// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Data/MagazineData.h"

UMagazineData::UMagazineData()
{
	ItemType = EItemType::Ammo;
	bStackable = true;
	MaxStackSize = 1;
}

bool UMagazineData::IsCompatibleWith(FGameplayTag OtherAmmoType) const
{
	// 정확히 같은 타입이면 호환
	if (AmmoType.MatchesTagExact(OtherAmmoType))
		return true;

	// 호환 가능 목록에 있으면 호환
	return CompatibleAmmoTypes.HasTagExact(OtherAmmoType);
}
