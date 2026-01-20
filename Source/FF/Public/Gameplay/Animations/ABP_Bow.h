// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ABP_Bow.generated.h"

/**
 * 
 */
UCLASS()
class FF_API UABP_Bow : public UAnimInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category = "Bow")
	float DrawBow;
};
