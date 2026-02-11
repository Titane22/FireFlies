// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Data/WeaponData.h"
#include "Curves/CurveFloat.h"

UWeaponData::UWeaponData()
{
    // Base ItemData defaults
    ItemName = FText::FromString(TEXT("Weapon"));
    bStackable = false;  // Weapons are not stackable
    MaxStackSize = 1;
	ItemType = EItemType::Equipment;

    // Weapon Details
    EquipmentClass = nullptr;

    // Fire Mode Data
    FireMode = EFireMode::SemiAuto;
    BurstAmount = 1;
    FireRate = 0.1f;  // 분당 발사 수

    // Ballistics
    BulletSpread = 2.0f;
    Damage = 0.0f;
    MaxRange = 5000.0f;  // 100m
    PelletsPerShot = 1;

    // UI
    WeaponUITexture = nullptr;

    // Animation
    WeaponEquipMontage = nullptr;
    BodyFireMontage = nullptr;
    WeaponFireMontage = nullptr;
    BodyReloadMontage = nullptr;
    WeaponReloadMontage = nullptr;

    // Audio
    FireSound = nullptr;
}

float UWeaponData::GetDamageAtDistance(float InBaseDamage, float Distance) const
{
    if (!DamageFalloffCurve)
        return InBaseDamage;
    float Multiplier = DamageFalloffCurve->GetFloatValue(Distance);
    return InBaseDamage * FMath::Clamp(Multiplier, 0.f, 1.f);
}

FPrimaryAssetId UWeaponData::GetPrimaryAssetId() const
{
    return FPrimaryAssetId("Weapon_C", GetFName());
}
