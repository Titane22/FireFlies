// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimationState.generated.h"

UENUM(BlueprintType)
enum class EAnimationState : uint8
{
	Unarmed     UMETA(DisplayName = "Unarmed"),
	Pistol      UMETA(DisplayName = "Pistol"),
	RifleShotgun UMETA(DisplayName = "Rifle/Shotgun")
};

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Forward     UMETA(DisplayName = "Forward"),
	Right		UMETA(DisplayName = "Right"),
	Backward	UMETA(DisplayName = "Backward"),
	Left		UMETA(DisplayName = "Left")
};

UENUM(BlueprintType)
enum class ELandState : uint8
{
	Normal		UMETA(DisplayName = "Normal"),
	Soft		UMETA(DisplayName = "Soft"),
	Hard		UMETA(DisplayName = "Hard")
};