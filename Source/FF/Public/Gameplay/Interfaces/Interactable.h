// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Gameplay/Data/InteractionContext.h"
#include "Interactable.generated.h"

class UInteractionData;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * IInteractable - 상호작용 가능한 모든 액터가 구현하는 인터페이스
 * AInteraction 클래스를 대체하여 더 유연한 상호작용 시스템 제공
 */
class FF_API IInteractable
{
	GENERATED_BODY()

public:
	/**
	 * 상호작용 실행
	 * @param Context - 상호작용 컨텍스트
	 * @return 상호작용 결과
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FInteractionResult ExecuteInteraction(const FInteractionContext& Context);

	/**
	 * 상호작용 가능 여부 체크
	 * @param InstigatorRef - 상호작용을 요청한 컨트롤러
	 * @return 상호작용 가능 여부
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(AController* InstigatorRef) const;

	/**
	 * 상호작용 프롬프트 텍스트 가져오기 (예: "Press E to pick up M4A1")
	 * @return 프롬프트 텍스트
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractionPrompt() const;

	/**
	 * Hold 타입 상호작용인지 체크
	 * @return true면 Hold 타입, false면 즉시 실행
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool IsHoldInteraction() const;

	/**
	 * Hold 지속 시간 가져오기 (IsHoldInteraction이 true일 때만 사용)
	 * @return Hold 지속 시간 (초)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	float GetHoldDuration() const;

	/**
	 * 상호작용이 한 번만 가능한지 체크
	 * @return true면 한 번 사용 후 비활성화
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool IsSingleUse() const;

	/**
	 * 하이라이트 상태 설정 (플레이어가 볼 때)
	 * @param bHighlight - 하이라이트 여부
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void SetHighlighted(bool bHighlight);

	/**
	 * 상호작용 시작 (Hold 타입용)
	 * @param Context - 상호작용 컨텍스트
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnInteractionStarted(const FInteractionContext& Context);

	/**
	 * 상호작용 취소 (Hold 타입용)
	 * @param Context - 상호작용 컨텍스트
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void OnInteractionCancelled(const FInteractionContext& Context);

	/**
	 * 상호작용 대상 액터 가져오기
	 * @return 액터 포인터
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	AActor* GetInteractableActor();
};
