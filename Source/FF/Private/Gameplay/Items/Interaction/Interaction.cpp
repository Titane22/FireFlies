// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Items/Interaction/Interaction.h"

#include "Components/WidgetComponent.h"
#include "Gameplay/Data/InteractionData.h"

AInteraction::AInteraction()
{
	PrimaryActorTick.bCanEverTick = false;

	// State
	bIsHighlighted = false;

	// BaseMesh를 RootComponent로 설정
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(FName("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	InteractionWidget = CreateDefaultSubobject<UWidgetComponent>(FName("InteractionWidget"));
	InteractionWidget->SetupAttachment(DefaultSceneRoot);
}

void AInteraction::BeginPlay()
{
	Super::BeginPlay();

	// 데이터 유효성 검증
	if (!InteractionData)
	{
		UE_LOG(LogTemp, Error, TEXT("%s: InteractionData is null! Please assign an InteractionData asset."), *GetName());
	}
}

//==============================================================================
// IInteractable Interface Implementation
//==============================================================================

FInteractionResult AInteraction::ExecuteInteraction_Implementation(const FInteractionContext& Context)
{
	// 기본 구현: 이벤트 브로드캐스트
	OnInteractionExecuted.Broadcast(this, Context);
	return FInteractionResult::Success();
}

bool AInteraction::CanInteract_Implementation(AController* InstigatorRef) const
{
	if (!InstigatorRef)
	{
		return false;
	}

	if (!InteractionData)
	{
		return true;
	}

	return InteractionData->bIsEnabled;
}

FText AInteraction::GetInteractionPrompt_Implementation() const
{
	if (InteractionData)
	{
		return InteractionData->PromptText;
	}
	return FText::FromString("Interact");
}

bool AInteraction::IsHoldInteraction_Implementation() const
{
	if (InteractionData)
	{
		return InteractionData->HoldDuration > 0.0f;
	}
	return false;
}

float AInteraction::GetHoldDuration_Implementation() const
{
	if (InteractionData)
	{
		return InteractionData->HoldDuration;
	}
	return 0.0f;
}

bool AInteraction::IsSingleUse_Implementation() const
{
	if (InteractionData)
	{
		return InteractionData->bSingleUse;
	}
	return false;
}

void AInteraction::SetHighlighted_Implementation(bool bHighlight)
{
	if (bIsHighlighted != bHighlight)
	{
		bIsHighlighted = bHighlight;
		OnHighlightChanged.Broadcast(bHighlight);
	}
}

void AInteraction::OnInteractionStarted_Implementation(const FInteractionContext& Context)
{
	OnInteractionStartedEvent.Broadcast(this, Context);
}

void AInteraction::OnInteractionCancelled_Implementation(const FInteractionContext& Context)
{
	// 기본 구현: 아무것도 하지 않음
}

AActor* AInteraction::GetInteractableActor_Implementation()
{
	return this;
}

EInteractiveType AInteraction::GetInteractionType_Implementation() const
{
	if (InteractionData)
	{
		return InteractionData->InteractionType;
	}
	return EInteractiveType::Default;
}
