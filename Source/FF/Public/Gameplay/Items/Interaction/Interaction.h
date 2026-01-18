// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/Data/InteractionContext.h"
#include "Gameplay/Interfaces/Interactable.h"
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
 * IInteractable 인터페이스의 기본 구현을 제공하며, InteractionData를 통해 설정을 관리
 */
UCLASS(Blueprintable)
class FF_API AInteraction : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	AInteraction();

protected:
	//==============================================================================
	// State
	//==============================================================================

	/** 현재 상호작용이 하이라이트되어 있는지 (플레이어가 보고 있는지) */
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bIsHighlighted = false;

	virtual void BeginPlay() override;
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* DefaultSceneRoot = nullptr;
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
	// IInteractable Interface Implementation
	//==============================================================================

	virtual FInteractionResult ExecuteInteraction_Implementation(const FInteractionContext& Context) override;
	virtual bool CanInteract_Implementation(AController* InstigatorRef) const override;
	virtual FText GetInteractionPrompt_Implementation() const override;
	virtual bool IsHoldInteraction_Implementation() const override;
	virtual float GetHoldDuration_Implementation() const override;
	virtual bool IsSingleUse_Implementation() const override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;
	virtual void OnInteractionStarted_Implementation(const FInteractionContext& Context) override;
	virtual void OnInteractionCancelled_Implementation(const FInteractionContext& Context) override;
	virtual AActor* GetInteractableActor_Implementation() override;
	virtual EInteractiveType GetInteractionType_Implementation() const override;
};
