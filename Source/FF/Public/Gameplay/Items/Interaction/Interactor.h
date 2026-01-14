// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Gameplay/Library/InteractiveType.h"
#include "Gameplay/Data/InteractionContext.h"
#include "Interactor.generated.h"

class APlayer_Base;
class AInteraction;
class UInteractionData;

/**
 * Interaction Detection Method
 */
UENUM(BlueprintType)
enum class EInteractionMethod : uint8
{
	Camera          UMETA(DisplayName = "Camera", Tooltip = "카메라 시점에서 전방으로 감지"),
	BodyForward		UMETA(DisplayName = "Body Forward", Tooltip = "캐릭터 본체 전방으로 감지")
};

/**
 * UInteractor - Data-Driven 상호작용 감지 컴포넌트
 * 안전성과 확장성을 고려하여 재설계됨
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FF_API UInteractor : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractor();

protected:
	//==============================================================================
	// Configuration
	//==============================================================================
	
	/** 상호작용 감지 거리 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction|Detection", meta = (ClampMin = "0.0"))
	float DetectionDistance = 350.0f;

	/** 감지 구체 반경 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction|Detection", meta = (ClampMin = "0.0"))
	float SphereTraceRadius = 10.0f;

	/** 감지 방법 (Camera / BodyForward) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction|Detection")
	EInteractionMethod InteractionMethod = EInteractionMethod::Camera;

	/** 감지 활성화 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction|Detection")
	bool bInteractorActive = true;

	/** 디버그 트레이스 표시 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction|Debug")
	bool bShowDebugTrace = false;

	/** 감지 업데이트 주기 (초) - 0이면 매 프레임 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction|Performance", meta = (ClampMin = "0.0"))
	float DetectionUpdateInterval = 0.0f;

	//==============================================================================
	// State
	//==============================================================================

	/** 현재 감지된 상호작용 대상 (IInteractable 인터페이스 구현 액터) */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction|State")
	TWeakObjectPtr<AActor> CurrentInteractionActor;

	/** 상호작용 진행 중 여부 (Hold 타입용) */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction|State")
	bool bIsInteracting = false;

	/** Hold 타입 상호작용 시작 시간 */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction|State")
	float InteractionStartTime = 0.0f;

	/** 마지막 감지 업데이트 시간 */
	float LastDetectionTime = 0.0f;

	//==============================================================================
	// References
	//==============================================================================

	/** 캐릭터 참조 (자동 설정) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction|References")
	APlayer_Base* CharacterRef = nullptr;

	//==============================================================================
	// Lifecycle
	//==============================================================================

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	//==============================================================================
	// Detection
	//==============================================================================
	
	/** 상호작용 감지 로직 */
	void DetectInteractions();

	/** 트레이스 시작/끝 위치 계산 */
	bool GetTraceStartEnd(FVector& OutStart, FVector& OutEnd) const;

	/** 새로운 상호작용 감지 시작 */
	void StartNewInteraction(AActor* NewInteractionActor);

public:
	//==============================================================================
	// Interaction Execution
	//==============================================================================

	/**
	 * 상호작용 시작
	 * Hold 타입이면 OnInteractionStarted 호출, 아니면 즉시 ExecuteInteraction
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TriggerInteraction();

	/**
	 * Hold 타입 상호작용 취소
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void CancelInteraction();

	/**
	 * 상호작용 실행 (내부용)
	 */
	void ExecuteCurrentInteraction();

	/** 현재 상호작용 하이라이트 중지 */
	void StopCurrentInteraction();
	//==============================================================================
	// Getters
	//==============================================================================

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool HasActiveInteraction() const { return CurrentInteractionActor.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetCurrentInteractionActor() const { return CurrentInteractionActor.Get(); }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	UInteractionData* GetCurrentInteractionData() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	EInteractiveType GetCurrentInteractionType() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsHoldInteraction() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	float GetHoldProgress() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	FText GetCurrentInteractionPrompt() const;

	//==============================================================================
	// Setters
	//==============================================================================

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractorActive(bool bActive) { bInteractorActive = bActive; }

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetDetectionDistance(float NewDistance) { DetectionDistance = FMath::Max(0.0f, NewDistance); }
};
