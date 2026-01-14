// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Items/Interaction/Interactor.h"
#include "Gameplay/Items/Interaction/Interaction.h"
#include "Gameplay/Interfaces/Interactable.h"
#include "Gameplay/Data/InteractionData.h"
#include "Gameplay/Characters/Player_Base.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

UInteractor::UInteractor()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Configuration
	DetectionDistance = 350.0f;
	SphereTraceRadius = 10.0f;
	InteractionMethod = EInteractionMethod::Camera;
	bInteractorActive = true;
	bShowDebugTrace = false;
	DetectionUpdateInterval = 0.0f;

	// State
	bIsInteracting = false;
	InteractionStartTime = 0.0f;
	LastDetectionTime = 0.0f;
}

void UInteractor::BeginPlay()
{
	Super::BeginPlay();

	// 캐릭터 참조 초기화
	CharacterRef = Cast<APlayer_Base>(GetOwner());
	if (!CharacterRef)
	{
		UE_LOG(LogTemp, Error, TEXT("UInteractor: Owner is not APlayer_Base! Component will not function."));
		SetComponentTickEnabled(false);
	}
}

void UInteractor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 비활성화 또는 캐릭터 없으면 스킵
	if (!bInteractorActive || !CharacterRef)
		return;

	// 업데이트 주기 체크 (성능 최적화)
	if (DetectionUpdateInterval > 0.0f)
	{
		if (GetWorld()->GetTimeSeconds() - LastDetectionTime < DetectionUpdateInterval)
			return;

		LastDetectionTime = GetWorld()->GetTimeSeconds();
	}

	// Hold 타입 상호작용 진행 중이면 완료 체크
	if (bIsInteracting && IsHoldInteraction())
	{
		const float HoldProgress = GetHoldProgress();
		if (HoldProgress >= 1.0f)
		{
			// Hold 완료
			ExecuteCurrentInteraction();
			bIsInteracting = false;
		}
		return; // Hold 중에는 새로운 감지 안 함
	}

	// 상호작용 감지
	DetectInteractions();
}

//==============================================================================
// Detection
//==============================================================================

void UInteractor::DetectInteractions()
{
	if (!CharacterRef || !GetWorld())
		return;

	// 트레이스 시작/끝 위치 계산
	FVector StartLocation, EndLocation;
	if (!GetTraceStartEnd(StartLocation, EndLocation))
	{
		StopCurrentInteraction();
		return;
	}

	// Sphere Trace 실행
	FHitResult HitResult;
	const bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		StartLocation,
		EndLocation,
		SphereTraceRadius,
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
		false,
		TArray<AActor*>{ CharacterRef },
		bShowDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		HitResult,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		0.1f
	);

	if (!bHit)
	{
		// 아무것도 감지 안 됨
		StopCurrentInteraction();
		return;
	}
	AActor* HitActor = HitResult.GetActor();
	if (!HitActor)
	{
		StopCurrentInteraction();
		return;
	}

	// IInteractable 인터페이스 구현 여부 체크
	if (!HitActor->Implements<UInteractable>())
	{
		// IInteractable 인터페이스를 구현하지 않음
		StopCurrentInteraction();
		return;
	}

	// 상호작용 가능 여부 체크
	AController* PC = CharacterRef->GetController();
	IInteractable* Interactable = Cast<IInteractable>(HitActor);
	if (!PC || !Interactable || !IInteractable::Execute_CanInteract(HitActor, PC))
	{
		// 상호작용 불가능
		StopCurrentInteraction();
		return;
	}

	// 이미 같은 대상이면 유지
	if (CurrentInteractionActor.IsValid() && CurrentInteractionActor.Get() == HitActor)
		return;

	// 새로운 상호작용 시작
	StopCurrentInteraction();
	StartNewInteraction(HitActor);
}

bool UInteractor::GetTraceStartEnd(FVector& OutStart, FVector& OutEnd) const
{
	if (!CharacterRef)
		return false;

	switch (InteractionMethod)
	{
	case EInteractionMethod::Camera:
		{
			APlayerCameraManager* PCM = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
			if (!PCM)
			{
				UE_LOG(LogTemp, Warning, TEXT("UInteractor: PlayerCameraManager is null!"));
				return false;
			}
			
			OutStart = PCM->GetCameraLocation();
			OutEnd = OutStart + (PCM->GetActorForwardVector() * DetectionDistance);
			return true;
		}

	case EInteractionMethod::BodyForward:
		{
			OutStart = CharacterRef->GetActorLocation();
			OutEnd = OutStart + (CharacterRef->GetActorForwardVector() * DetectionDistance);
			return true;
		}

	default:
		return false;
	}
}

void UInteractor::StopCurrentInteraction()
{
	if (!CurrentInteractionActor.IsValid())
		return;

	// 하이라이트 해제
	AActor* Actor = CurrentInteractionActor.Get();
	if (Actor && Actor->Implements<UInteractable>())
	{
		IInteractable::Execute_SetHighlighted(Actor, false);
	}

	CurrentInteractionActor.Reset();
}

void UInteractor::StartNewInteraction(AActor* NewInteractionActor)
{
	if (!NewInteractionActor || !NewInteractionActor->Implements<UInteractable>())
		return;

	CurrentInteractionActor = NewInteractionActor;
	IInteractable::Execute_SetHighlighted(NewInteractionActor, true);

	UE_LOG(LogTemp, Log, TEXT("New interaction detected: %s"), *NewInteractionActor->GetName());
}

//==============================================================================
// Interaction Execution
//==============================================================================

void UInteractor::TriggerInteraction()
{
	if (!CurrentInteractionActor.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("TriggerInteraction: No current interaction!"));
		return;
	}

	AActor* InteractionActor = CurrentInteractionActor.Get();
	if (!InteractionActor || !InteractionActor->Implements<UInteractable>())
		return;

	AController* PC = CharacterRef ? CharacterRef->GetController() : nullptr;
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("TriggerInteraction: Player controller is null!"));
		return;
	}

	// Hold 타입 체크
	bool bIsHold = IInteractable::Execute_IsHoldInteraction(InteractionActor);
	if (bIsHold)
	{
		// Hold 시작
		bIsInteracting = true;
		InteractionStartTime = GetWorld()->GetTimeSeconds();

		FInteractionContext Context;
		Context.InstigatorRef = PC;
		Context.InstigatorPawn = CharacterRef;

		IInteractable::Execute_OnInteractionStarted(InteractionActor, Context);

		UE_LOG(LogTemp, Log, TEXT("Hold interaction started: %s"), *InteractionActor->GetName());
	}
	else
	{
		// 즉시 실행
		ExecuteCurrentInteraction();
	}
}

void UInteractor::CancelInteraction()
{
	if (!bIsInteracting || !CurrentInteractionActor.IsValid())
		return;

	AActor* InteractionActor = CurrentInteractionActor.Get();
	if (!InteractionActor || !InteractionActor->Implements<UInteractable>())
		return;

	AController* PC = CharacterRef ? CharacterRef->GetController() : nullptr;
	if (!PC)
		return;

	bIsInteracting = false;
	InteractionStartTime = 0.0f;

	FInteractionContext Context;
	Context.InstigatorRef = PC;
	Context.InstigatorPawn = CharacterRef;

	IInteractable::Execute_OnInteractionCancelled(InteractionActor, Context);

	UE_LOG(LogTemp, Log, TEXT("Interaction cancelled: %s"), *InteractionActor->GetName());
}

void UInteractor::ExecuteCurrentInteraction()
{
	if (!CurrentInteractionActor.IsValid())
		return;

	AActor* InteractionActor = CurrentInteractionActor.Get();
	if (!InteractionActor || !InteractionActor->Implements<UInteractable>())
		return;

	AController* PC = CharacterRef ? CharacterRef->GetController() : nullptr;
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("ExecuteCurrentInteraction: Player controller is null!"));
		return;
	}

	// 거리 계산
	float Distance = 0.0f;
	if (CharacterRef)
	{
		Distance = FVector::Dist(CharacterRef->GetActorLocation(), InteractionActor->GetActorLocation());
	}

	// Context 생성 및 실행
	FInteractionContext Context;
	Context.InstigatorRef = PC;
	Context.InstigatorPawn = CharacterRef;
	Context.Distance = Distance;

	FInteractionResult Result = IInteractable::Execute_ExecuteInteraction(InteractionActor, Context);

	if (Result.bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Interaction succeeded: %s"), *InteractionActor->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Interaction failed: %s - %s"),
			*InteractionActor->GetName(),
			*Result.FailureReason.ToString());
	}

	// 상태 리셋
	bIsInteracting = false;
	InteractionStartTime = 0.0f;

	// SingleUse면 자동으로 하이라이트 해제
	bool bIsSingleUse = IInteractable::Execute_IsSingleUse(InteractionActor);
	if (bIsSingleUse)
	{
		StopCurrentInteraction();
	}
}

//==============================================================================
// Getters
//==============================================================================

UInteractionData* UInteractor::GetCurrentInteractionData() const
{
	// Legacy support - returns InteractionData if actor provides it
	if (!CurrentInteractionActor.IsValid())
		return nullptr;

	AActor* Actor = CurrentInteractionActor.Get();
	if (!Actor || !Actor->Implements<UInteractable>())
		return nullptr;

	return IInteractable::Execute_GetInteractionData(Actor);
}

EInteractiveType UInteractor::GetCurrentInteractionType() const
{
	// Try to get from InteractionData first (legacy support)
	UInteractionData* Data = GetCurrentInteractionData();
	if (Data)
		return Data->InteractionType;

	// Default
	return EInteractiveType::Default;
}

bool UInteractor::IsHoldInteraction() const
{
	if (!CurrentInteractionActor.IsValid())
		return false;

	AActor* Actor = CurrentInteractionActor.Get();
	if (!Actor || !Actor->Implements<UInteractable>())
		return false;

	return IInteractable::Execute_IsHoldInteraction(Actor);
}

float UInteractor::GetHoldProgress() const
{
	if (!bIsInteracting || !IsHoldInteraction() || !GetWorld())
		return 0.0f;

	if (!CurrentInteractionActor.IsValid())
		return 0.0f;

	AActor* Actor = CurrentInteractionActor.Get();
	if (!Actor || !Actor->Implements<UInteractable>())
		return 0.0f;

	float HoldDuration = IInteractable::Execute_GetHoldDuration(Actor);
	if (HoldDuration <= 0.0f)
		return 0.0f;

	const float ElapsedTime = GetWorld()->GetTimeSeconds() - InteractionStartTime;
	return FMath::Clamp(ElapsedTime / HoldDuration, 0.0f, 1.0f);
}

FText UInteractor::GetCurrentInteractionPrompt() const
{
	if (!CurrentInteractionActor.IsValid())
		return FText::GetEmpty();

	AActor* Actor = CurrentInteractionActor.Get();
	if (!Actor || !Actor->Implements<UInteractable>())
		return FText::GetEmpty();

	return IInteractable::Execute_GetInteractionPrompt(Actor);
}
