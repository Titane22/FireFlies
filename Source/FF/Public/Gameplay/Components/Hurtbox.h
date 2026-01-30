// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "Hurtbox.generated.h"

UENUM(BlueprintType)
enum class EMeleeDirection : uint8
{
	Left	UMETA(DisplayName = "Left"),
	Right	UMETA(DisplayName = "Right"),
	Both	UMETA(DisplayName = "Both"),
	Top		UMETA(DisplayName = "Top")
};

class AFFCharacter;
class UPhysicalAnimationComponent;
class USkeletalMeshComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FF_API UHurtbox : public UActorComponent
{
	GENERATED_BODY()

public:
	UHurtbox();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	AFFCharacter* CharacterRef = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Hurtbox")
	float GetDamageMultiplier(FName BoneName) const;

	/** 피격 시 호출 - Physical Animation 트리거 + Impulse */
	UFUNCTION(BlueprintCallable, Category = "Hurtbox")
	void TriggerHitReaction(const FVector& HitLocation, const FVector& HitDirection, FName BoneName, float ImpulseStrength = 500.f);

	UFUNCTION(BlueprintCallable, Category = "HitReaction")
	void MeleeHitReaction(const FVector& HitLocation, const FVector& HitDirection);
protected:
	virtual void BeginPlay() override;

private:
	void DisablePhysicalAnimation(float DeltaTime);

	UFUNCTION()
	void PATrigger();

	UPROPERTY()
	USkeletalMeshComponent* CachedMesh = nullptr;

	UPROPERTY()
	UPhysicalAnimationComponent* CachedPAC = nullptr;

	// 상태 변수
	bool bDisablePAC = false;
	bool bDoOnceCompleted = false;
	bool bFlipFlopState = false;  // FlipFlop: false=A, true=B
	float CurrentBlendWeight = 1.f;

	FTimerHandle PATriggerTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Hurtbox|Config")
	float BlendInterpSpeed = 10.f;

	UPROPERTY(EditAnywhere, Category = "Hurtbox|Config")
	float TriggerDelay = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Hurtbox|HitReaction")
	TMap<EMeleeDirection, UAnimMontage*> HitReactionMontages;

	EMeleeDirection GetHitDirection(const FVector& HitDirection) const;
};
