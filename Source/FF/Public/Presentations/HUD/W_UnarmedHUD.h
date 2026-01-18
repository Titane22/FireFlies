// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "W_UnarmedHUD.generated.h"

class UImage;
class UTextBlock;
class UW_CircularProgressBar;

/**
 * Unarmed HUD widget containing crosshair and interaction progress bar
 * Displayed when player has no weapon equipped or during hold interactions
 */
UCLASS()
class FF_API UW_UnarmedHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	//==========================================================================
	// Crosshair
	//==========================================================================

	/** Show/Hide the crosshair */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Crosshair")
	void SetCrosshairVisible(bool bVisible);

	/** Set crosshair color (e.g., change when targeting interactable) */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Crosshair")
	void SetCrosshairColor(FLinearColor Color);

	//==========================================================================
	// Interaction Progress
	//==========================================================================

	/** Show interaction progress bar (called when hold interaction starts) */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
	void ShowInteractionProgress();

	/** Hide interaction progress bar */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
	void HideInteractionProgress();

	/** Update interaction progress (0.0 ~ 1.0) */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
	void UpdateInteractionProgress(float Progress);

	/** Set interaction prompt text (e.g., "[Hold E] Swap weapons") */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
	void SetInteractionPrompt(const FText& PromptText);

	/** Called when interaction is completed successfully */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
	void OnInteractionCompleted();

	/** Called when interaction is cancelled */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Interaction")
	void OnInteractionCancelled();

protected:
	/** Crosshair image (center dot) */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Components")
	UImage* CrosshairImage;

	/** Circular progress bar widget */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Components")
	UW_CircularProgressBar* InteractionProgressBar;

	/** Interaction prompt text block */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Components")
	UTextBlock* PromptTextBlock;
};
