// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h"
#include "InteractiveType.generated.h"


UENUM(BlueprintType)
enum class EInteractiveType : uint8
{
	Default			UMETA(DisplayName = "Default"),
	Pickup			UMETA(DisplayName = "Pickup"),
	WeaponPickup	UMETA(DisplayName = "WeaponPickup")
};

/**
 * 
 */
UCLASS()
class FF_API UInteractiveType : public UUserDefinedEnum
{
	GENERATED_BODY()
	
};
