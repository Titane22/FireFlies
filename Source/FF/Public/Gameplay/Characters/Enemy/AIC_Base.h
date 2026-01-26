// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Gameplay/Interfaces/EnemyState.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "AIC_Base.generated.h"

UENUM(BlueprintType)
enum class ESenseType : uint8
{
	None		UMETA(DisplayName = "None"),
	Sight		UMETA(DisplayName = "Sight"),
	Hearing		UMETA(DisplayName = "Hearing"),
	Damage		UMETA(DisplayName = "Damage")
};

UENUM(BlueprintType)
enum class EAIState : uint8
{
	Passive			UMETA(DisplayName = "Passive"),
	Attacking		UMETA(DisplayName = "Attacking"),
	Investigating	UMETA(DisplayName = "Investigating"),
	Seeking			UMETA(DisplayName = "Seeking")
};
/**
 * 
 */
UCLASS()
class FF_API AAIC_Base : public AAIController, public IEnemyState
{
	GENERATED_BODY()
public:
	AAIC_Base();

	virtual void OnPossess(APawn* InPawn);

	void SetStateAsPassive();

	void SetStateAsAttacking(AActor* ToSetAttackTarget, bool bUseLastKnownAttackTarget);

	void SetStateAsInvestigating(FVector Location);
	
	bool CanSenseActor(AActor* Actor, ESenseType SenseType, FAIStimulus& StimulusRef);
	
	void HandleSensedSight(AActor* Actor);

	void HandleLostSight(AActor* Actor);
	
	void HandleSensedHearing(FVector Location);

	void HandleSensedDamage(AActor* Actor);

	void SeekAttackTarget();
	
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdateActors);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	UAIPerceptionComponent* AIPerception;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	UAISenseConfig_Hearing* HearingConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	UAISenseConfig_Damage* DamageConfig;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Perception")
	AActor* AttackTarget;
	
	UPROPERTY(BlueprintReadOnly, Category = "AI|Perception")
	FVector LastKnownLocation;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Perception")
	float AwarenessLevel = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	FName AttackTargetKeyName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	FName InvestigateLocationKeyName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	FName StateKeyName;
	
	EAIState AIState;

	FTimerHandle SeekAttackTargetTimer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	float TimeToSeekAfterLosingSight = 3.f;
};
