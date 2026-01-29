// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Animations/Notify/ANS_MeleeCombo.h"

#include "Gameplay/Characters/FFCharacter.h"
#include "Gameplay/Items/Equipments/MasterWeapon.h"
#include "Gameplay/Items/Equipments/MeleeAttackSystem.h"

UANS_WeaponTrace::UANS_WeaponTrace()
{
	TraceChannel = ECC_Pawn;
	bDrawDebug = true;
	DebugDrawTime = 2.0f;
	DebugLineThickness = 3.0f;
}

void UANS_WeaponTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
		return;

	AFFCharacter* Character = Cast<AFFCharacter>(MeshComp->GetOwner());
	if (!Character)
		return;

	AMasterWeapon* CurrentWeapon = Character->GetCurrentWeapon();
	if (!CurrentWeapon)
		return;
	
	UMeleeAttackSystem* CombatComp = Cast<UMeleeAttackSystem>(CurrentWeapon->AttackSystem);
	if (!CombatComp)
		return;

	
	// 무기 Sweep 시작
	CombatComp->StartWeaponSweep();

	UE_LOG(LogTemp, Log, TEXT("[ANS_WeaponTrace] Weapon sweep started for %s"), *Character->GetName());
}

void UANS_WeaponTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
		return;

	AFFCharacter* Character = Cast<AFFCharacter>(MeshComp->GetOwner());
	if (!Character)
		return;

	AMasterWeapon* CurrentWeapon = Character->GetCurrentWeapon();
	if (!CurrentWeapon)
		return;
	
	UMeleeAttackSystem* CombatComp = Cast<UMeleeAttackSystem>(CurrentWeapon->AttackSystem);
	if (!CombatComp)
		return;

	UWorld* World = Character->GetWorld();
	if (!World)
		return;

	// 무기의 Trace 포인트 가져오기
	FVector Start, End;
	if (!CombatComp->GetWeaponTracePoints(Start, End))
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANS_WeaponTrace] Failed to get weapon trace points"));
		return;
	}

	float Radius = CombatComp->GetWeaponSweepRadius();

	// Sphere Sweep Trace 수행
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);
	QueryParams.bTraceComplex = false;

	bool bHit = World->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		TraceChannel,
		FCollisionShape::MakeSphere(Radius),
		QueryParams
	);

	// 디버그 드로우
	if (bDrawDebug)
	{
		// 무기 궤적 라인
		DrawDebugLine(
			World,
			Start,
			End,
			FColor::Red,
			false,
			DebugDrawTime,
			0,
			DebugLineThickness
		);

		// 시작점 구체
		DrawDebugSphere(
			World,
			Start,
			Radius,
			12,
			FColor::Green,
			false,
			DebugDrawTime,
			0,
			1.0f
		);

		// 끝점 구체
		DrawDebugSphere(
			World,
			End,
			Radius,
			12,
			FColor::Blue,
			false,
			DebugDrawTime,
			0,
			1.0f
		);

		// 히트 지점 표시
		if (bHit)
		{
			DrawDebugSphere(
				World,
				HitResult.ImpactPoint,
				Radius * 1.5f,
				12,
				FColor::Yellow,
				false,
				DebugDrawTime,
				0,
				2.0f
			);
		}
	}

	// 히트 처리
	if (bHit)
	{
		CombatComp->OnWeaponHit(HitResult);

		UE_LOG(LogTemp, Log, TEXT("[ANS_WeaponTrace] Hit: %s at %s"),
			*HitResult.GetActor()->GetName(),
			*HitResult.ImpactPoint.ToString());
	}
}

void UANS_WeaponTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
		return;

	AFFCharacter* Character = Cast<AFFCharacter>(MeshComp->GetOwner());
	if (!Character)
		return;

	AMasterWeapon* CurrentWeapon = Character->GetCurrentWeapon();
	if (!CurrentWeapon)
		return;
	
	UMeleeAttackSystem* CombatComp = Cast<UMeleeAttackSystem>(CurrentWeapon->AttackSystem);
	if (!CombatComp)
		return;

	// 무기 Sweep 종료
	CombatComp->StopWeaponSweep();

	UE_LOG(LogTemp, Log, TEXT("[ANS_WeaponTrace] Weapon sweep stopped for %s"), *Character->GetName());
}