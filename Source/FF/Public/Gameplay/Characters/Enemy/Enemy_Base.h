// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Characters/FFCharacter.h"
#include "Enemy_Base.generated.h"

/**
 * 
 */
UCLASS()
class FF_API AEnemy_Base : public AFFCharacter
{
	GENERATED_BODY()

protected:
	AEnemy_Base();
	virtual void BeginPlay();
};
