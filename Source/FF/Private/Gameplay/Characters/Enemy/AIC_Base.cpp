// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Characters/Enemy/AIC_Base.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Gameplay/Characters/Player_Base.h"
#include "Slate/SGameLayerManager.h"

AAIC_Base::AAIC_Base()
{
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>("AI Perception");
	SetPerceptionComponent(*AIPerception);

	// Sight Configuration - 시각 설정
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("Sight Config");
	SightConfig->SightRadius = 1200.0f;  // 기본 시야 거리
	SightConfig->LoseSightRadius = 1500.0f;  // 시야를 잃는 거리
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;  // 주변 시야각 (180도가 전체)
	SightConfig->SetMaxAge(5.0f);  // 마지막으로 본 후 기억 유지 시간
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;  // 마지막 위치 근처에서 자동 탐지 거리
	AIPerception->ConfigureSense(*SightConfig);

	// Hearing Configuration - 청각 설정
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>("Hearing Config");
	HearingConfig->HearingRange = 2000.0f;  // 소리 감지 거리
	HearingConfig->SetMaxAge(3.0f);  // 소리 기억 유지 시간
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	AIPerception->ConfigureSense(*HearingConfig);

	// Damage Configuration - 피해 감지 설정
	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>("Damage Config");
	DamageConfig->SetMaxAge(2.0f);  // 피해 기억 유지 시간
	AIPerception->ConfigureSense(*DamageConfig);

	// 주요 감각을 시각으로 설정
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());

	InvestigateLocationKeyName = FName("InvestigateLocation");
	AttackTargetKeyName = FName("AttackTarget");
	StateKeyName = FName("State");
}

void AAIC_Base::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (AIPerception)
	{
		AIPerception->OnPerceptionUpdated.AddDynamic(this, &AAIC_Base::OnPerceptionUpdated);
	}
}

void AAIC_Base::SetStateAsPassive()
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		AIState = EAIState::Passive;
		BB->SetValueAsEnum(StateKeyName, static_cast<uint8>(EAIState::Passive));
	}
}

void AAIC_Base::SetStateAsAttacking(AActor* ToSetAttackTarget, bool bUseLastKnownAttackTarget)
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		AActor* NewAttackTarget = nullptr;
		if (AttackTarget && bUseLastKnownAttackTarget)
		{
			NewAttackTarget = AttackTarget;
		}
		else
		{
			NewAttackTarget = ToSetAttackTarget;
		}

		if (!NewAttackTarget)
		{
			SetStateAsPassive();
			return;
		}
		AIState = EAIState::Attacking;
		BB->SetValueAsEnum(StateKeyName, static_cast<uint8>(EAIState::Attacking));
		BB->SetValueAsObject(AttackTargetKeyName, NewAttackTarget);
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow,
			FString::Printf(TEXT("ToSetAttackTarget: %s"), *NewAttackTarget->GetName()));

		AttackTarget = NewAttackTarget;
	}
}

void AAIC_Base::SetStateAsInvestigating(FVector Location)
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString::Printf(TEXT("ToSetAttackTarget: %s"), *Location.ToString()));
		AIState = EAIState::Investigating;
		BB->SetValueAsEnum(StateKeyName, static_cast<uint8>(EAIState::Investigating));
		BB->SetValueAsVector(InvestigateLocationKeyName, Location);
	}
}

bool AAIC_Base::CanSenseActor(AActor* Actor, ESenseType SenseType, FAIStimulus& StimulusRef)
{
	if (!Actor || !AIPerception)
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor or AI Perception is NULL"));
		return false;
	}

	FActorPerceptionBlueprintInfo Info;
	AIPerception->GetActorsPerception(Actor, Info);

	for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
	{
		// Stimulus의 실제 Sense 타입 확인
		TSubclassOf<UAISense> SenseClass = UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus);

		bool bIsCorrectSense = false;
		if (SenseType == ESenseType::Sight && SenseClass == UAISense_Sight::StaticClass())
		{
			bIsCorrectSense = true;
		}
		else if (SenseType == ESenseType::Hearing && SenseClass == UAISense_Hearing::StaticClass())
		{
			bIsCorrectSense = true;
		}
		else if (SenseType == ESenseType::Damage && SenseClass == UAISense_Damage::StaticClass())
		{
			bIsCorrectSense = true;
		}

		if (bIsCorrectSense && Stimulus.WasSuccessfullySensed())
		{
			StimulusRef = Stimulus;
			return true;
		}
	}
	return false;
}

void AAIC_Base::HandleSensedSight(AActor* Actor)
{
	switch (AIState)
	{
	case EAIState::Passive:
	case EAIState::Investigating:
		SetStateAsAttacking(Actor, false);
		break;
	case EAIState::Attacking:
		GetWorldTimerManager().ClearTimer(SeekAttackTargetTimer);
		break;
	}
}

void AAIC_Base::HandleLostSight(AActor* Actor)
{
	if (Actor == AttackTarget)
	{
		switch (AIState)
		{
		case EAIState::Attacking:
		case EAIState::Investigating:
			GetWorldTimerManager().ClearTimer(SeekAttackTargetTimer);
			GetWorldTimerManager().SetTimer(
				SeekAttackTargetTimer,
				this,
				&AAIC_Base::SeekAttackTarget,
				TimeToSeekAfterLosingSight,
				false
			);
			break;
		}
	}
}

void AAIC_Base::HandleSensedHearing(FVector Location)
{
	switch (AIState)
	{
	case EAIState::Attacking:
	case EAIState::Investigating:
	case EAIState::Seeking:
		SetStateAsInvestigating(Location);
		break;
	}
}

void AAIC_Base::HandleSensedDamage(AActor* Actor)
{
	switch (AIState)
	{
	case EAIState::Passive:
	case EAIState::Investigating:
	case EAIState::Seeking:
		SetStateAsAttacking(Actor, false);
		break;
	}
}

void AAIC_Base::SeekAttackTarget()
{
	if (!AttackTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SeekAttackTarget] - Attack Target is null"))
		return;
	}
	FVector Location = AttackTarget->GetActorLocation();
	SetStateAsInvestigating(Location);
	GetWorldTimerManager().ClearTimer(SeekAttackTargetTimer);
	
}

void AAIC_Base::OnPerceptionUpdated(const TArray<AActor*>& UpdateActors)
{
	// GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
	// 	FString::Printf(TEXT("OnPerceptionUpdated called with %d actors"), UpdateActors.Num()));

	for (AActor* Actor : UpdateActors)
	{
		APlayer_Base* Player = Cast<APlayer_Base>(Actor);
		if (!Player)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange,
				FString::Printf(TEXT("Actor %s is not Player_Base, skipping"), Actor ? *Actor->GetName() : TEXT("NULL")));
			continue;
		}

		float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Player->GetActorLocation());

		FAIStimulus SightStimulus;
		FAIStimulus HearingStimulus;
		FAIStimulus DamageStimulus;
		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString::Printf(TEXT("Distance: %lf"), Distance));
		// 시각 감지 체크
		if (CanSenseActor(Player, ESenseType::Sight, SightStimulus))
		{
			// 성공적으로 시야에 들어옴 - 거리에 따른 반응
			if (Distance < 300.f)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("Called"));
				// 가까운 거리 - 즉시 공격
				HandleSensedSight(Player);
			}
			else if (Distance < 800.f)
			{
				// 중간 거리 - 조사 또는 추적
				if (AIState == EAIState::Passive)
				{
					SetStateAsInvestigating(Player->GetActorLocation());
				}
				else
				{
					// 이미 활성 상태면 공격으로 전환
					HandleSensedSight(Player);
				}
			}
			else
			{
				// 먼 거리 - 계속 추적 (이미 공격 중이면 유지)
				if (AIState == EAIState::Attacking)
				{
					HandleSensedSight(Player);
				}
			}
			LastKnownLocation = SightStimulus.StimulusLocation;
		}
		else
		{
			// 시야를 잃음
			FActorPerceptionBlueprintInfo Info;
			AIPerception->GetActorsPerception(Player, Info);

			// 이전에 감지했던 적이 있는지 확인
			for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
			{
				TSubclassOf<UAISense> SenseClass = UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus);
				if (SenseClass == UAISense_Sight::StaticClass())
				{
					if (!Stimulus.WasSuccessfullySensed())
					{
						// 시야를 잃었을 때만 처리
						HandleLostSight(Player);
						LastKnownLocation = Stimulus.StimulusLocation;
					}
					break;
				}
			}
		}

		// 청각 감지 체크
		if (CanSenseActor(Player, ESenseType::Hearing, HearingStimulus))
		{
			if (HearingStimulus.Strength > 0.7f || Distance < 400.f)
			{
				// 큰 소리 또는 가까운 소리 - 정확한 위치로 조사
				HandleSensedSight(Player);
			}
			else if (HearingStimulus.Strength > 0.4f || Distance < 1000.f)
			{
				// 작은 소리 또는 먼 소리 - 자극 위치로 조사
				HandleSensedHearing(HearingStimulus.StimulusLocation);
			}
		}

	}
}
