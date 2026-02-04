// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Components/Hurtbox.h"
#include "Gameplay/Characters/FFCharacter.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameplayTagContainer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "Components/SkeletalMeshComponent.h"

UHurtbox::UHurtbox()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHurtbox::BeginPlay()
{
	Super::BeginPlay();

	// 약간의 딜레이 후 초기화 (컴포넌트들이 준비된 후)
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			if (CharacterRef)
			{
				CachedMesh = CharacterRef->GetMesh();
				CachedPAC = CharacterRef->GetPAC();

				// if (CachedPAC && CachedMesh)
				// {
				// 	CachedPAC->SetSkeletalMeshComponent(CachedMesh);
				//
				// 	FPhysicalAnimationData PhysicalAnimationData;
				// 	PhysicalAnimationData.bIsLocalSimulation = false;
				// 	PhysicalAnimationData.OrientationStrength = 500.f;
				// 	PhysicalAnimationData.AngularVelocityStrength = 100.f;
				// 	PhysicalAnimationData.PositionStrength = 500.f;
				// 	PhysicalAnimationData.VelocityStrength = 100.f;
				// 	PhysicalAnimationData.MaxLinearForce = 0.f;
				// 	PhysicalAnimationData.MaxAngularForce = 0.f;
				// 	CachedPAC->ApplyPhysicalAnimationSettingsBelow(FName("spine_02"), PhysicalAnimationData, false);
				//
				// 	CachedMesh->SetAllBodiesBelowSimulatePhysics(FName("spine_02"), true, true);
				// }
			}
		},
		0.1f,
		false
	);
}

void UHurtbox::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Branch: bDisablePAC가 true면 DisablePhysicalAnimation 실행
	if (bDisablePAC)
	{
		DisablePhysicalAnimation(DeltaTime);
	}
}

void UHurtbox::DisablePhysicalAnimation(float DeltaTime)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, TEXT("Called"));
	if (!CachedMesh)
		return;

	// PhysicsBlendWeight를 1에서 0으로 서서히 감소
	CurrentBlendWeight = FMath::FInterpTo(CurrentBlendWeight, 0.f, DeltaTime, BlendInterpSpeed);
	CachedMesh->SetAllBodiesBelowPhysicsBlendWeight(FName("spine_02"), CurrentBlendWeight, false, false);
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Called"));
	// Do Once: BlendWeight가 거의 0에 도달하면 타이머 설정
	if (CurrentBlendWeight <= 0.01f && !bDoOnceCompleted)
	{
		bDoOnceCompleted = true;
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, TEXT("Called"));
		// Set Timer by Function Name - 0.2초 후 PATrigger 호출
		GetWorld()->GetTimerManager().SetTimer(
			PATriggerTimerHandle,
			this,
			&UHurtbox::PATrigger,
			TriggerDelay,
			false  // Looping = false
		);
	}
}

void UHurtbox::PATrigger()
{
	// Flip Flop 로직 (먼저 체크, 나중에 토글)
	if (!bFlipFlopState)
	{
		// A 경로 (첫 번째 호출): Disable PA = true
		bDisablePAC = true;
	}
	else
	{
		// B 경로 (두 번째 호출): Disable PA = false
		bDisablePAC = false;
	}

	bFlipFlopState = !bFlipFlopState;  // 토글은 마지막에
	bDoOnceCompleted = false;
}

void UHurtbox::TriggerHitReaction(const FVector& HitLocation, const FVector& HitDirection, FName BoneName, float ImpulseStrength)
{
	if (!CachedMesh)
		return;

	// 물리 시뮬레이션 활성화 (Impulse 적용에 필수)
	CachedMesh->SetAllBodiesBelowSimulatePhysics(FName("spine_02"), true, true);

	// BlendWeight 리셋 (물리 영향 최대)
	CurrentBlendWeight = 1.f;
	CachedMesh->SetAllBodiesBelowPhysicsBlendWeight(FName("spine_02"), 1.f, false, true);

	// Impulse 적용
	FVector Impulse = HitDirection.GetSafeNormal() * ImpulseStrength;
	CachedMesh->AddImpulseAtLocation(Impulse, HitLocation, BoneName);

	// PATrigger 호출 → bDisablePAC = true → Tick에서 BlendWeight 감소 시작
	PATrigger();
}

void UHurtbox::ApplyHitStagger()
{
	if (!CharacterRef)
		return;

	const FGameplayTag HitReactTag = FGameplayTag::RequestGameplayTag(FName("State.Disabled.HitReacting"));
	CharacterRef->AddStateTag(HitReactTag);

	if (UCharacterMovementComponent* MoveComp = CharacterRef->GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}
}

void UHurtbox::EndHitStagger()
{
	if (!CharacterRef)
		return;

	const FGameplayTag HitReactTag = FGameplayTag::RequestGameplayTag(FName("State.Disabled.HitReacting"));
	CharacterRef->RemoveStateTag(HitReactTag);

	if (UCharacterMovementComponent* MoveComp = CharacterRef->GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}
}

void UHurtbox::MeleeHitReaction(const FVector& HitLocation, const FVector& HitDirection)
{
	if (!CharacterRef)
		return;

	ApplyHitStagger();

	EMeleeDirection Dir = GetHitDirection(HitDirection);
	UAnimMontage** FoundMontage = HitReactionMontages.Find(Dir);

	if (FoundMontage && *FoundMontage)
	{
		if (UAnimInstance* Anim = CharacterRef->GetMesh()->GetAnimInstance())
		{
			Anim->Montage_Play(*FoundMontage, 0.5f);

			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &UHurtbox::OnHitReactionEnded);
			Anim->Montage_SetEndDelegate(EndDelegate, *FoundMontage);
		}
	}
	else
	{
		// 몽타주 없음 - 타이머로 경직 해제
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(HitStaggerTimerHandle);
			World->GetTimerManager().SetTimer(
				HitStaggerTimerHandle,
				this,
				&UHurtbox::EndHitStagger,
				HitStaggerDuration,
				false
			);
		}
		else
		{
			// World 없음 - 즉시 해제
			EndHitStagger();
		}
	}
}

void UHurtbox::OnHitReactionEnded(UAnimMontage* Montage, bool bInterrupted)
{
	EndHitStagger();
}

EMeleeDirection UHurtbox::GetHitDirection(const FVector& HitDirection) const
{
	if (!CharacterRef)
		return EMeleeDirection::Both;

	FVector Forward = CharacterRef->GetActorForwardVector();
	FVector Right = CharacterRef->GetActorRightVector();
	FVector HitDir2D = FVector(HitDirection.X, HitDirection.Y, 0.f).GetSafeNormal();

	float DotForward = FVector::DotProduct(Forward, HitDir2D);
	float DotRight = FVector::DotProduct(Right, HitDir2D);

	// 위에서 내려치는 공격 (Z 성분이 큰 경우)
	if (HitDirection.Z < -0.5f)
		return EMeleeDirection::Top;

	// 좌/우가 전/후보다 강하면 좌우 판별
	if (FMath::Abs(DotRight) > FMath::Abs(DotForward))
	{
		return DotRight > 0.f ? EMeleeDirection::Right : EMeleeDirection::Left;
	}

	// 전/후방 → Both
	return EMeleeDirection::Both;
}

//==============================================================================
// 데미지 배율
//==============================================================================

float UHurtbox::GetDamageMultiplier(FName BoneName) const
{
	return 1.f;
}
