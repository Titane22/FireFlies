// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Library/InteractiveType.h"
#include "InteractionComponent.generated.h"

/**
 * @deprecated UInteractionComponent is deprecated. Use UInteractionData (DataAsset) instead.
 * This component is kept for backward compatibility only and will be removed in future versions.
 *
 * Migration guide:
 * 1. Create a new InteractionData asset in the Content Browser
 * 2. Assign it to AInteraction::InteractionData
 * 3. Remove this component from your Blueprint/Actor
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Deprecated)
class FF_API UDEPRECATED_InteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDEPRECATED_InteractionComponent();

	/** @deprecated Use InteractionData instead */
	UE_DEPRECATED(5.0, "Use UInteractionData DataAsset instead")
	EInteractiveType InteractionType = EInteractiveType::Default;
};
