// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LootingSystem.generated.h"

struct FLootItemRow;
class UInventorySystem;
class UDataTable;

/**
 * 컨테이너별 루트 설정
 */
USTRUCT(BlueprintType)
struct FLootContainerSettings
{
	GENERATED_BODY()

	/** 루트 테이블 (FLootItemRow) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UDataTable* LootTable = nullptr;

	/** 컨테이너 최소 가치 (이 범위의 아이템만 드롭) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ContainerMinValue = 0;

	/** 컨테이너 최대 가치 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ContainerMaxValue = 100;

	/** 최소 드롭 아이템 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MinDropCount = 1;

	/** 최대 드롭 아이템 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxDropCount = 5;
};

/**
 * 루팅 시스템 컴포넌트
 * DataTable 기반으로 루트 생성
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FF_API ULootingSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	ULootingSystem();

	/** 루트 생성 */
	UFUNCTION(BlueprintCallable, Category = "Looting")
	void GenerateLoot();

	/** 루팅 가능 여부 */
	UFUNCTION(BlueprintPure, Category = "Looting")
	bool CanLooting() const { return bLootingActive; }

	/** 루팅 활성화 설정 */
	UFUNCTION(BlueprintCallable, Category = "Looting")
	void SetLootActive(bool bActive) { bLootingActive = bActive; }

	/** 인벤토리 시스템 설정 */
	void SetInventorySystem(UInventorySystem* IS);

protected:
	virtual void BeginPlay() override;

	/** 가중치 기반 아이템 선택 */
	const FLootItemRow* SelectByWeight(const TArray<const FLootItemRow*>& Entries);

protected:
	/** 연결된 인벤토리 시스템 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Looting")
	UInventorySystem* OwnerIS = nullptr;

	/** 루트 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Looting")
	FLootContainerSettings LootSettings;

	/** 루팅 활성화 상태 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Looting")
	bool bLootingActive = false;
};
