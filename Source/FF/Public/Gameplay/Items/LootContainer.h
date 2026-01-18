// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Items/Interaction/Interaction.h"
#include "LootContainer.generated.h"

/*
 * 루트 컨테이너
 * AInteraction을 상속하여 InteractionData 기반의 상호작용 기능을 제공
 */

class ULootingSystem;
class UInventorySystem;

UCLASS()
class FF_API ALootContainer : public AInteraction
{
	GENERATED_BODY()

public:
	ALootContainer();

protected:
	virtual void BeginPlay() override;

public:
	//==============================================================================
	// IInteractable Interface Override (컨테이너 전용 로직)
	//==============================================================================

	virtual FInteractionResult ExecuteInteraction_Implementation(const FInteractionContext& Context) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UInventorySystem* InventorySystem = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	ULootingSystem* LootingSystem = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	bool bGenerateLootOnFirstOpen = true;

	UPROPERTY(BlueprintReadOnly, Category = "Container")
	bool bHasGeneratedLoot = false;
};
