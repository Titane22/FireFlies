// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Data/InteractionContext.h"
#include "Interaction.generated.h"

class UInteractionData;
class UWidgetComponent;

// 무기 탄약 상태 저장용 구조체
USTRUCT(BlueprintType)
struct FWeaponAmmoState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 CurrentAmmo = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 MaxAmmo = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 ClipAmmo = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 DifferentAmmo = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 AmmoCount = 0;

	// 기본 생성자
	FWeaponAmmoState()
		: CurrentAmmo(0), MaxAmmo(0), ClipAmmo(0), DifferentAmmo(0), AmmoCount(0)
	{}
};

// 상호작용 이벤트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionExecuted, AInteraction*, Interaction, const FInteractionContext&, Context);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionStarted, AInteraction*, Interaction, const FInteractionContext&, Context);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionHighlightChanged, bool, bHighlighted);

/**
 * AInteraction - Data-Driven 상호작용 베이스 클래스
 * 더 이상 자식 클래스를 만들 필요 없이, InteractionData와 Event로 모든 것을 처리
 */
UCLASS(Blueprintable)
class FF_API AInteraction : public AActor
{
	GENERATED_BODY()

public:
	AInteraction();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* DefaultSceneRoot = nullptr;

	//==============================================================================
	// State
	//==============================================================================

	/** 현재 상호작용이 하이라이트되어 있는지 (플레이어가 보고 있는지) */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction", Replicated)
	bool bIsHighlighted = false;

	/** 마지막 상호작용 시간 (쿨다운 계산용) */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	float LastInteractionTime = 0.0f;

	//==============================================================================
	// Lifecycle
	//==============================================================================

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	//==============================================================================
	// Events (Data-Driven)
	//==============================================================================

	/** 상호작용 실행 시 발생 - 블루프린트/C++에서 바인딩 가능 */
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionExecuted OnInteractionExecuted;

	/** 상호작용 시작 시 발생 (Hold 타입용) */
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionStarted OnInteractionStartedEvent;

	/** 하이라이트 상태 변경 시 발생 */
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionHighlightChanged OnHighlightChanged;

	//==============================================================================
	// Interaction Data (Data-Driven)
	//==============================================================================

	/** 이 상호작용의 모든 설정을 담고 있는 DataAsset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	UInteractionData* InteractionData = nullptr;

	//==============================================================================
	// Weapon State (드롭된 무기용)
	//==============================================================================

	/** 드롭된 무기의 탄약 상태 (bHasCustomState가 true일 때만 사용) */
	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	FWeaponAmmoState SavedAmmoState;

	/** 커스텀 탄약 상태를 가지고 있는지 (false면 기본 WeaponData 사용) */
	UPROPERTY(BlueprintReadWrite, Category = "Weapon")
	bool bHasCustomState = false;

public:
	
	//==============================================================================
	// Interaction Interface
	//==============================================================================

	/**
	 * 상호작용 실행 (자식 클래스에서 오버라이드)
	 * @param Context - 상호작용 컨텍스트
	 * @return 상호작용 결과
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	FInteractionResult ExecuteInteraction(const FInteractionContext& Context);
	virtual FInteractionResult ExecuteInteraction_Implementation(const FInteractionContext& Context);

	/**
	 * 상호작용 가능 여부 체크
	 * @param InstigatorRef - 상호작용을 요청한 컨트롤러
	 * @return 상호작용 가능 여부
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(AController* InstigatorRef) const;
	virtual bool CanInteract_Implementation(AController* InstigatorRef) const;

	/**
	 * 하이라이트 상태 설정 (플레이어가 볼 때)
	 * @param bHighlight - 하이라이트 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void SetHighlighted(bool bHighlight);

	/**
	 * 상호작용 시작 (Hold 타입용)
	 * @param Context - 상호작용 컨텍스트
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void OnInteractionStarted(const FInteractionContext& Context);
	virtual void OnInteractionStarted_Implementation(const FInteractionContext& Context);

	/**
	 * 상호작용 취소 (Hold 타입용)
	 * @param Context - 상호작용 컨텍스트
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void OnInteractionCancelled(const FInteractionContext& Context);
	virtual void OnInteractionCancelled_Implementation(const FInteractionContext& Context);

	//==============================================================================
	// Getters
	//==============================================================================

	UFUNCTION(BlueprintPure, Category = "Interaction")
	UInteractionData* GetInteractionData() const { return InteractionData; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsHighlighted() const { return bIsHighlighted; }

	/** 쿨다운 중인지 확인 */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsOnCooldown() const;

	/** 남은 쿨다운 시간 */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	float GetRemainingCooldown() const;

	//==============================================================================
	// Deprecated (Backward Compatibility)
	//==============================================================================

	/** @deprecated Use ExecuteInteraction instead */
	UE_DEPRECATED(5.0, "Use ExecuteInteraction instead")
	virtual void Interact(AController* Interactor);
};
