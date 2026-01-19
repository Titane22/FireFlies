// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Data/WeaponData.h"

UWeaponData::UWeaponData()
{
    // Base ItemData defaults
    ItemName = FText::FromString(TEXT("Weapon"));
    bStackable = false;  // Weapons are not stackable
    MaxStackSize = 1;

    // Weapon Details
    WeaponType = EWeaponType::Pistol;
    EquipmentClass = nullptr;

    // Fire Mode Data
    FireMode = EFireMode::SemiAuto;
    BurstAmount = 1;
    FireRate = 0.1f;  // 분당 발사 수

    // Ballistics
    BulletSpread = 2.0f;
    Damage = 0.0f;
    MaxRange = 5000.0f;  // 100m

    // UI
    WeaponUITexture = nullptr;

    // Animation
    LeftHandIKOffset = FVector::ZeroVector;
    WeaponEquipMontage = nullptr;
    BodyFireMontage = nullptr;
    WeaponFireMontage = nullptr;
    BodyReloadMontage = nullptr;
    WeaponReloadMontage = nullptr;

    // Audio
    FireSound = nullptr;
}

FPrimaryAssetId UWeaponData::GetPrimaryAssetId() const
{
    return FPrimaryAssetId("Weapon_C", GetFName());
}
