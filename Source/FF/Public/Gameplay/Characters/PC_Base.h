// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PC_Base.generated.h"

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

protected:
	virtual void BeginPlay();
};
