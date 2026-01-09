// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Gameplay/Library/InteractiveType.h"
#include "InteractionData.generated.h"

/**
 * Data-Driven Interaction Configuration
 * 디자이너가 블루프린트에서 다양한 상호작용 타입을 정의할 수 있게 함
 */
UCLASS(BlueprintType)
class FF_API UInteractionData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UInteractionData();

	//==============================================================================
	// Basic Configuration
	//==============================================================================

	/** 상호작용 타입 (Pickup, WeaponPickup, Door, Lever 등) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Basic")
	EInteractiveType InteractionType = EInteractiveType::Default;

	/** 상호작용 이름 (UI에 표시됨) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Basic")
	FText InteractionName = FText::FromString("Interact");

	/** 상호작용 설명 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Basic", meta = (MultiLine = true))
	FText InteractionDescription;

	/** 상호작용 가능 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Interaction|Basic")
	bool bIsEnabled = true;

	//==============================================================================
	// Detection Settings
	//==============================================================================

	/** 상호작용 감지 거리 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Detection", meta = (ClampMin = "0.0"))
	float DetectionRadius = 200.0f;

	/** 상호작용 유지 시간 (Hold 타입일 때) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Detection", meta = (ClampMin = "0.0"))
	float HoldDuration = 0.0f;

	/** 한 번만 상호작용 가능 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Detection")
	bool bSingleUse = false;

	/** 쿨다운 시간 (초) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Detection", meta = (ClampMin = "0.0"))
	float CooldownTime = 0.0f;

	//==============================================================================
	// Visual Feedback
	//==============================================================================

	/** 상호작용 프롬프트 텍스트 (예: "Press E to pick up") */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Visual")
	FText PromptText = FText::FromString("Press E to interact");

	/** 상호작용 아이콘 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Visual")
	UTexture2D* InteractionIcon = nullptr;

	/** 하이라이트 컬러 (상호작용 가능할 때) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Visual")
	FLinearColor HighlightColor = FLinearColor::Green;

	//==============================================================================
	// Audio Feedback
	//==============================================================================

	/** 상호작용 시작 사운드 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Audio")
	USoundBase* InteractionStartSound = nullptr;

	/** 상호작용 완료 사운드 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Audio")
	USoundBase* InteractionCompleteSound = nullptr;

	/** 상호작용 실패 사운드 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Audio")
	USoundBase* InteractionFailSound = nullptr;

	//==============================================================================
	// Requirements & Conditions
	//==============================================================================

	/** 상호작용 가능한 최대 거리 체크 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Requirements")
	bool bCheckDistance = true;

	//==============================================================================
	// Payload Data (타입별 데이터를 여기에 저장)
	//==============================================================================

	/**
	 * 페이로드 데이터 - 상호작용 타입에 따라 필요한 추가 데이터
	 * 예: WeaponPickup → Weapon Class
	 *     Door → Required Key ID
	 *     Lever → Target Actor Reference
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Payload")
	TMap<FString, FString> PayloadData;

	//==============================================================================
	// Helper Functions
	//==============================================================================

	/** 상호작용이 현재 사용 가능한지 체크 */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool CanInteract() const { return bIsEnabled; }

	/** Hold 타입 상호작용인지 체크 */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsHoldInteraction() const { return HoldDuration > 0.0f; }

	/** 프롬프트 텍스트 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	FText GetFormattedPrompt() const { return PromptText; }

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
