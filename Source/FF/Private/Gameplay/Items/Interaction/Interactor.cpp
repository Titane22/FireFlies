// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Items/Interaction/Interactor.h"
#include "Gameplay/Items/Interaction/Interaction.h"
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

	// AInteraction으로 캐스팅 시도
	AInteraction* HitInteraction = Cast<AInteraction>(HitResult.GetActor());
	if (!HitInteraction)
	{
		// Interaction이 아닌 다른 액터
		UE_LOG(LogTemp, Warning, TEXT("Hit actor is not AInteraction: %s"), *HitResult.GetActor()->GetName());
		StopCurrentInteraction();
		return;
	}

	// 상호작용 가능 여부 체크
	AController* PC = CharacterRef->GetController();
	if (!PC || !HitInteraction->CanInteract(PC))
	{
		// 상호작용 불가능
		UE_LOG(LogTemp, Warning, TEXT("CanInteract failed for: %s (PlayerController: %s)"),
			*HitInteraction->GetName(),
			PC ? TEXT("Valid") : TEXT("NULL"));
		StopCurrentInteraction();
		return;
	}

	// 이미 같은 대상이면 유지
	if (CurrentInteraction.IsValid() && CurrentInteraction.Get() == HitInteraction)
		return;

	// 새로운 상호작용 시작
	StopCurrentInteraction();
	StartNewInteraction(HitInteraction);
}

bool UInteractor::GetTraceStartEnd(FVector& OutStart, FVector& OutEnd) const
{
	if (!CharacterRef)
		return false;

	switch (InteractionMethod)
	{
	case EInteractionMethod::Camera:
		{
			UCameraComponent* Camera = CharacterRef->GetFollowCamera();
			if (!Camera)
			{
				UE_LOG(LogTemp, Warning, TEXT("UInteractor: FollowCamera is null!"));
				return false;
			}

			OutStart = Camera->GetComponentLocation();
			OutEnd = OutStart + (Camera->GetForwardVector() * DetectionDistance);
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
	if (!CurrentInteraction.IsValid())
		return;

	// 하이라이트 해제
	if (AInteraction* Interaction = CurrentInteraction.Get())
	{
		Interaction->SetHighlighted(false);
	}

	CurrentInteraction.Reset();
}

void UInteractor::StartNewInteraction(AInteraction* NewInteraction)
{
	if (!NewInteraction)
		return;

	CurrentInteraction = NewInteraction;
	NewInteraction->SetHighlighted(true);

	UE_LOG(LogTemp, Log, TEXT("New interaction detected: %s"), *NewInteraction->GetName());
}

//==============================================================================
// Interaction Execution
//==============================================================================

void UInteractor::TriggerInteraction()
{
	if (!CurrentInteraction.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("TriggerInteraction: No current interaction!"));
		return;
	}

	AInteraction* Interaction = CurrentInteraction.Get();
	if (!Interaction)
		return;

	AController* PC = CharacterRef ? CharacterRef->GetController() : nullptr;
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("TriggerInteraction: Player controller is null!"));
		return;
	}

	// Hold 타입 체크
	if (IsHoldInteraction())
	{
		// Hold 시작
		bIsInteracting = true;
		InteractionStartTime = GetWorld()->GetTimeSeconds();

		FInteractionContext Context(PC, Interaction, Interaction->GetInteractionData());
		Interaction->OnInteractionStarted(Context);

		UE_LOG(LogTemp, Log, TEXT("Hold interaction started: %s"), *Interaction->GetName());
	}
	else
	{
		// 즉시 실행
		ExecuteCurrentInteraction();
	}
}

void UInteractor::CancelInteraction()
{
	if (!bIsInteracting || !CurrentInteraction.IsValid())
		return;

	AInteraction* Interaction = CurrentInteraction.Get();
	if (!Interaction)
		return;

	AController* PC = CharacterRef ? CharacterRef->GetController() : nullptr;
	if (!PC)
		return;

	bIsInteracting = false;
	InteractionStartTime = 0.0f;

	FInteractionContext Context(PC, Interaction, Interaction->GetInteractionData());
	Interaction->OnInteractionCancelled(Context);

	UE_LOG(LogTemp, Log, TEXT("Interaction cancelled: %s"), *Interaction->GetName());
}

void UInteractor::ExecuteCurrentInteraction()
{
	if (!CurrentInteraction.IsValid())
		return;

	AInteraction* Interaction = CurrentInteraction.Get();
	if (!Interaction)
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
		Distance = FVector::Dist(CharacterRef->GetActorLocation(), Interaction->GetActorLocation());
	}

	// Context 생성 및 실행
	FInteractionContext Context(PC, Interaction, Interaction->GetInteractionData(), Distance);
	FInteractionResult Result = Interaction->ExecuteInteraction(Context);

	if (Result.bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Interaction succeeded: %s"), *Interaction->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Interaction failed: %s - %s"),
			*Interaction->GetName(),
			*Result.FailureReason.ToString());
	}

	// 상태 리셋
	bIsInteracting = false;
	InteractionStartTime = 0.0f;

	// SingleUse면 자동으로 하이라이트 해제
	UInteractionData* Data = Interaction->GetInteractionData();
	if (Data && Data->bSingleUse)
	{
		StopCurrentInteraction();
	}
}

//==============================================================================
// Getters
//==============================================================================

UInteractionData* UInteractor::GetCurrentInteractionData() const
{
	if (!CurrentInteraction.IsValid())
		return nullptr;

	AInteraction* Interaction = CurrentInteraction.Get();
	return Interaction ? Interaction->GetInteractionData() : nullptr;
}

EInteractiveType UInteractor::GetCurrentInteractionType() const
{
	UInteractionData* Data = GetCurrentInteractionData();
	return Data ? Data->InteractionType : EInteractiveType::Default;
}

bool UInteractor::IsHoldInteraction() const
{
	UInteractionData* Data = GetCurrentInteractionData();
	return Data ? Data->IsHoldInteraction() : false;
}

float UInteractor::GetHoldProgress() const
{
	if (!bIsInteracting || !IsHoldInteraction() || !GetWorld())
		return 0.0f;

	UInteractionData* Data = GetCurrentInteractionData();
	if (!Data || Data->HoldDuration <= 0.0f)
		return 0.0f;

	const float ElapsedTime = GetWorld()->GetTimeSeconds() - InteractionStartTime;
	return FMath::Clamp(ElapsedTime / Data->HoldDuration, 0.0f, 1.0f);
}
