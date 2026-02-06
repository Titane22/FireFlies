// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "W_InteractionScreen.generated.h"

class UInteractionData;
class UItemData;
/**
 * 
 */
UCLASS()
class FF_API UW_InteractionScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
	void SetItemInfo(UItemData* ItemData);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
	void SetInteractionInfo(UInteractionData* InteractionData);
protected:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Interaction")
	UTextBlock* ItemName;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Interaction")
	UTextBlock* ItemWeight;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Interaction")
	UTextBlock* ItemDescription;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Interaction")
	UBorder* BottomBorder;
};
