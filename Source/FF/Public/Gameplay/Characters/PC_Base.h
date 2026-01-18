// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "PC_Base.generated.h"

class UEquipmentSystem;
class UInventorySystem;
class APlayer_Base;
/**
 * 
 */
UCLASS()
class FF_API APC_Base : public APlayerController
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Config")
	APlayer_Base* OwnerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Config|Inventory")
	UInventorySystem* InventorySystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Config|Equipment")
	UEquipmentSystem* EquipmentSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Config|Inventory")
	UUserWidget* InvenWidget;

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OpenInventory();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
	void CloseInventory();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Inventory")
	void LootingInteract(UInventorySystem* ToLootContainerInventory);
	
	bool IsVisibleWidget() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetIgnoreInput(bool bIgnoreInput);
	// Gameplay Tags for Input Modes
	UPROPERTY(BlueprintReadWrite, Category = "Input")
	FGameplayTagContainer InputModeTags;

	UFUNCTION(BlueprintCallable, Category = "Input")
	void AddInputModeTag(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void RemoveInputModeTag(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "Input")
	bool HasInputModeTag(FGameplayTag Tag) const;
protected:
	virtual void BeginPlay();
};
