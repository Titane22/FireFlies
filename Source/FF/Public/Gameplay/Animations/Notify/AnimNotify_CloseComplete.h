// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_CloseComplete.generated.h"

/**
 * 
 */
UCLASS()
class FF_API UAnimNotify_CloseComplete : public UAnimNotify
{
	GENERATED_BODY()

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const                      
			FAnimNotifyEventReference& EventReference) override;
};
