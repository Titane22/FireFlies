// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Data/ItemData.h"
#include "GameplayTagContainer.h"
#include "MagazineData.generated.h"

/**
 * 탄창/탄약 데이터 에셋
 * 무기와 분리되어 개별 관리되는 탄약 시스템
 */
UCLASS()
class FF_API UMagazineData : public UItemData
{
	GENERATED_BODY()

public:
	UMagazineData();
	/** AssetManager용 PrimaryAssetId 반환 */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	//==============================================================================
	// Ammo Type
	//==============================================================================

	// 탄약 타입 태그 (Ammo.Rifle, Ammo.Pistol 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magazine|Type")
	FGameplayTag AmmoType;

	// 호환 가능한 탄약 태그들 (여러 타입 호환 시)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magazine|Type")
	FGameplayTagContainer CompatibleAmmoTypes;

	//==============================================================================
	// Capacity
	//==============================================================================

	// 탄창 용량 (한 탄창에 들어가는 탄약 수)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magazine|Capacity")
	int32 ClipSize = 30;

	//==============================================================================
	// Combat
	//==============================================================================

	// 데미지 배율 (탄약별 데미지 차이)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magazine|Combat")
	float DamageMultiplier = 1.0f;

	// 관통력 (0.0 ~ 1.0, 벽/적 관통)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magazine|Combat")
	float PenetrationPower = 0.0f;

	//==============================================================================
	// Visual & Audio
	//==============================================================================

	// 탄약 메쉬 (월드에 드롭될 때)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magazine|Visual")
	UStaticMesh* AmmoMesh;

	// 발사 사운드 오버라이드 (nullptr이면 무기 기본 사운드 사용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magazine|Audio")
	USoundBase* FireSoundOverride;

	//==============================================================================
	// Functions
	//==============================================================================

	// 탄약 타입 호환 확인
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	bool IsCompatibleWith(FGameplayTag OtherAmmoType) const;
};
