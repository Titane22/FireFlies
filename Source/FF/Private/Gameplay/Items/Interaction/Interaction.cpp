// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Items/Interaction/Interaction.h"
#include "Gameplay/Data/InteractionData.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

AInteraction::AInteraction()
{
	PrimaryActorTick.bCanEverTick = false;

	// Components
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	// Replication
	bReplicates = true;
	SetReplicateMovement(false);

	// State
	bIsHighlighted = false;
	LastInteractionTime = 0.0f;
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

void AInteraction::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AInteraction, bIsHighlighted);
}

//==============================================================================
// Interaction Interface Implementation
//==============================================================================

FInteractionResult AInteraction::ExecuteInteraction_Implementation(const FInteractionContext& Context)
{
	// 기본 구현: 상호작용 가능 여부만 체크
	if (!CanInteract(Context.InstigatorRef))
	{
		return FInteractionResult::Failure(FText::FromString("Cannot interact with this object"));
	}

	// 쿨다운 업데이트
	LastInteractionTime = GetWorld()->GetTimeSeconds();

	// 사운드 재생
	if (InteractionData && InteractionData->InteractionCompleteSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, InteractionData->InteractionCompleteSound, GetActorLocation());
	}

	// 이벤트 브로드캐스트 - 외부에서 처리 가능!
	OnInteractionExecuted.Broadcast(this, Context);

	UE_LOG(LogTemp, Log, TEXT("Interaction executed: %s"), *GetName());

	return FInteractionResult::Success();
}

bool AInteraction::CanInteract_Implementation(AController* InstigatorRef) const
{
	// 데이터 체크
	if (!InteractionData)
	{
		UE_LOG(LogTemp, Warning, TEXT("CanInteract: InteractionData is NULL for %s"), *GetName());
		return false;
	}

	// 비활성화된 상호작용
	if (!InteractionData->bIsEnabled)
	{
		UE_LOG(LogTemp, Warning, TEXT("CanInteract: %s is disabled"), *GetName());
		return false;
	}

	// 쿨다운 체크
	if (IsOnCooldown())
	{
		UE_LOG(LogTemp, Warning, TEXT("CanInteract: %s is on cooldown"), *GetName());
		return false;
	}


	return true;
}

void AInteraction::SetHighlighted(bool bHighlight)
{
	if (bIsHighlighted == bHighlight)
		return;

	bIsHighlighted = bHighlight;

	// 이벤트 브로드캐스트 - 블루프린트에서 시각 효과 추가 가능
	OnHighlightChanged.Broadcast(bHighlight);

	// 블루프린트에서 시각적 피드백 구현 가능
	// (예: Outline, Glow 효과 등)
}

void AInteraction::OnInteractionStarted_Implementation(const FInteractionContext& Context)
{
	// Hold 타입 상호작용 시작 시 호출
	if (InteractionData && InteractionData->InteractionStartSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, InteractionData->InteractionStartSound, GetActorLocation());
	}

	// 이벤트 브로드캐스트
	OnInteractionStartedEvent.Broadcast(this, Context);

	UE_LOG(LogTemp, Log, TEXT("Interaction started (Hold): %s"), *GetName());
}

void AInteraction::OnInteractionCancelled_Implementation(const FInteractionContext& Context)
{
	// Hold 타입 상호작용 취소 시 호출
	if (InteractionData && InteractionData->InteractionFailSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, InteractionData->InteractionFailSound, GetActorLocation());
	}

	UE_LOG(LogTemp, Log, TEXT("Interaction cancelled: %s"), *GetName());
}

//==============================================================================
// Getters
//==============================================================================

bool AInteraction::IsOnCooldown() const
{
	if (!InteractionData || InteractionData->CooldownTime <= 0.0f)
		return false;

	if (!GetWorld())
		return false;

	const float TimeSinceLastInteraction = GetWorld()->GetTimeSeconds() - LastInteractionTime;
	return TimeSinceLastInteraction < InteractionData->CooldownTime;
}

float AInteraction::GetRemainingCooldown() const
{
	if (!IsOnCooldown())
		return 0.0f;

	if (!GetWorld() || !InteractionData)
		return 0.0f;

	const float TimeSinceLastInteraction = GetWorld()->GetTimeSeconds() - LastInteractionTime;
	return FMath::Max(0.0f, InteractionData->CooldownTime - TimeSinceLastInteraction);
}

//==============================================================================
// Deprecated
//==============================================================================

void AInteraction::Interact(AController* Interactor)
{
	// Backward compatibility
	FInteractionContext Context(Interactor, this, InteractionData);
	ExecuteInteraction(Context);
}

