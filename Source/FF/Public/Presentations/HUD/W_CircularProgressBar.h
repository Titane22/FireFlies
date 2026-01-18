// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "W_CircularProgressBar.generated.h"

class UImage;
class UMaterialInstanceDynamic;

/**
 * Circular progress bar widget (donut style like BOTW stamina)
 * Used for hold interactions
 */
UCLASS()
class FF_API UW_CircularProgressBar : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set progress value (0.0 ~ 1.0) */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Progress")
	void SetProgress(float Progress);

	/** Show/Hide the progress bar */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Progress")
	void SetProgressVisible(bool bVisible);

	/** Set progress bar color */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Progress")
	void SetProgressColor(FLinearColor Color);

	/** Play completion animation */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Progress")
	void PlayCompletionAnimation();

	/** Reset progress to 0 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Progress")
	void ResetProgress();

protected:
	/** Progress bar image using circular material */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional), Category = "Components")
	UImage* CircularProgressImage;

	/** Dynamic material instance for progress control */
	UPROPERTY(BlueprintReadWrite, Category = "Progress")
	UMaterialInstanceDynamic* ProgressMaterial;

	/** Base material for creating dynamic instance */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Progress")
	UMaterialInterface* ProgressMaterialBase;

	/** Current progress value */
	UPROPERTY(BlueprintReadWrite, Category = "Progress")
	float CurrentProgress = 0.0f;
};
