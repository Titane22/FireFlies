// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Data/LootTableData.h"
#include "LootingSystem.generated.h"

class UInventorySystem;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FF_API ULootingSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULootingSystem();

	UFUNCTION(BlueprintCallable, Category = "Interaction|Events")
	void GenerateLoot();

	bool CanLooting() const { return bLootingActive; }

	void SetLootActive(bool bActive) { bLootingActive = bActive; }

	void SetInventorySystem(UInventorySystem* IS);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	const FLootItemEntry* SelectByWeight(TArray<const FLootItemEntry*> Entries);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInventorySystem* OwnerIS = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loot")
	ULootTableData* LootTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loot")
	bool bLootingActive = false;

};
