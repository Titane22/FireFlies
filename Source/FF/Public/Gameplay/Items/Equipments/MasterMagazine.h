// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MasterMagazine.generated.h"

class UMagazineData;

/**
 * 탄창 런타임 인스턴스 (UObject 기반)
 * 무기에 장착된 탄창의 실제 상태를 관리
 *
 * TODO: 시각적 재장전 애니메이션이 필요하면 Actor 기반으로 확장
 *       - 재장전 시 손에 들고 있는 탄창 메쉬
 *       - 탄창 드롭 시 월드에 스폰
 *       - 무기 소켓에 어태치된 탄창 시각화

UCLASS()
class FF_API AMasterMagazine : public AEquipmentBase
{
	GENERATED_BODY()

public:
	AMasterMagazine();

protected:
	virtual void BeginPlay() override;

public:
	//==============================================================================
	// Magazine Data
	//==============================================================================

	// 탄창 데이터 에셋 (용량, 호환 탄종 등 정적 데이터) 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine")
	UMagazineData* MagazineData;

	//==============================================================================
	// Runtime State
	//==============================================================================

	// 현재 탄약 수 
	UPROPERTY(BlueprintReadWrite, Category = "Magazine|State")
	int32 CurrentAmmo = 0;

	//==============================================================================
	// Ammo Functions
	//==============================================================================

	// 탄약 소비 (발사 시 호출) - 실제 소비된 수 반환 
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	int32 ConsumeAmmo(int32 Amount = 1);

	// 탄약 추가 - 실제 추가된 수 반환 
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	int32 AddAmmo(int32 Amount);

	// 탄약이 있는지 확인 
	UFUNCTION(BlueprintPure, Category = "Magazine")
	bool HasAmmo() const { return CurrentAmmo > 0; }

	// 탄창이 비었는지 확인
	UFUNCTION(BlueprintPure, Category = "Magazine")
	bool IsEmpty() const { return CurrentAmmo <= 0; }

	// 탄창이 가득 찼는지 확인 
	UFUNCTION(BlueprintPure, Category = "Magazine")
	bool IsFull() const;

	// 탄창 용량 반환 (MagazineData에서) 
	UFUNCTION(BlueprintPure, Category = "Magazine")
	int32 GetClipSize() const;

	// 현재 탄약 수 반환 
	UFUNCTION(BlueprintPure, Category = "Magazine")
	int32 GetCurrentAmmo() const { return CurrentAmmo; }

	// 탄창을 가득 채움 
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	void FillMagazine();

	//==============================================================================
	// Initialization
	//==============================================================================

	// MagazineData로부터 탄창 초기화 (스폰 시 호출) 
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	void InitializeFromData(UMagazineData* InMagazineData, int32 InitialAmmo = -1);

	// 특정 탄약 수로 초기화 (-1이면 가득 채움) 
	void SetInitialAmmo(int32 Amount);
};
*/

UCLASS(BlueprintType)
class FF_API UMasterMagazine : public UObject
{
	GENERATED_BODY()

public:
	UMasterMagazine();

	//==============================================================================
	// Magazine Data
	//==============================================================================

	/** 탄창 데이터 에셋 (용량, 호환 탄종 등 정적 데이터) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine")
	UMagazineData* MagazineData = nullptr;

	//==============================================================================
	// Runtime State
	//==============================================================================

	/** 현재 탄약 수 */
	UPROPERTY(BlueprintReadWrite, Category = "Magazine|State")
	int32 CurrentAmmo = 0;

	//==============================================================================
	// Ammo Functions
	//==============================================================================

	/** 탄약 소비 (발사 시 호출) - 실제 소비된 수 반환 */
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	int32 ConsumeAmmo(int32 Amount = 1);

	/** 탄약 추가 - 실제 추가된 수 반환 */
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	int32 AddAmmo(int32 Amount);

	/** 탄약이 있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Magazine")
	bool HasAmmo() const { return CurrentAmmo > 0; }

	/** 탄창이 비었는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Magazine")
	bool IsEmpty() const { return CurrentAmmo <= 0; }

	/** 탄창이 가득 찼는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Magazine")
	bool IsFull() const;

	/** 탄창 용량 반환 (MagazineData에서) */
	UFUNCTION(BlueprintPure, Category = "Magazine")
	int32 GetClipSize() const;

	/** 현재 탄약 수 반환 */
	UFUNCTION(BlueprintPure, Category = "Magazine")
	int32 GetCurrentAmmo() const { return CurrentAmmo; }

	/** 탄창을 가득 채움 */
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	void FillMagazine();

	//==============================================================================
	// Initialization
	//==============================================================================

	/** MagazineData로부터 탄창 초기화 */
	UFUNCTION(BlueprintCallable, Category = "Magazine")
	void InitializeFromData(UMagazineData* InMagazineData, int32 InitialAmmo = -1);

	/** 특정 탄약 수로 초기화 (-1이면 가득 채움) */
	void SetInitialAmmo(int32 Amount);
};
