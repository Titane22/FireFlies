// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Items/Interaction/InteractionComponent.h"

// Sets default values for this component's properties
UDEPRECATED_InteractionComponent::UDEPRECATED_InteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	InteractionType = EInteractiveType::Default;
}

