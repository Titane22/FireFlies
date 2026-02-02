// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Animations/Notify/ANS_MeleeCombo.h"

#include "Gameplay/Characters/FFCharacter.h"
#include "Gameplay/Items/Equipments/MeleeAttackSystem.h"
#include "Kismet/KismetSystemLibrary.h"

UANS_WeaponTrace::UANS_WeaponTrace()
{
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

	UMeleeAttackSystem* CombatComp = Character->GetMeleeAttackSystem();
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

	UMeleeAttackSystem* CombatComp = Character->GetMeleeAttackSystem();
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

	// 무기 블레이드 전체를 커버하는 Sphere Overlap
	FVector Mid = (Start + End) * 0.5f;
	float OverlapRadius = FVector::Dist(Start, End) * 0.5f + Radius;

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Character);

	TArray<AActor*> OverlappedActors;
	UKismetSystemLibrary::SphereOverlapActors(
		World,
		Mid,
		OverlapRadius,
		ObjectTypes,
		nullptr,
		IgnoreActors,
		OverlappedActors
	);
	
	// 디버그 드로우
	if (bDrawDebug)
	{
		DrawDebugLine(World, Start, End, FColor::Red, false, DebugDrawTime, 0, DebugLineThickness);
		DrawDebugSphere(World, Start, Radius, 12, FColor::Green, false, DebugDrawTime, 0, 1.0f);
		DrawDebugSphere(World, End, Radius, 12, FColor::Blue, false, DebugDrawTime, 0, 1.0f);
		DrawDebugSphere(World, Mid, OverlapRadius, 12, FColor::Cyan, false, DebugDrawTime, 0, 0.5f);
	}

	// 히트 처리
	for (AActor* HitActor : OverlappedActors)
	{
		if (CombatComp->OnWeaponHit(HitActor))
		{
			if (bDrawDebug)
			{
				DrawDebugSphere(World, HitActor->GetActorLocation(), Radius * 1.5f, 12, FColor::Yellow, false, DebugDrawTime, 0, 2.0f);
			}

			UE_LOG(LogTemp, Log, TEXT("[ANS_WeaponTrace] Hit: %s"), *HitActor->GetName());
		}
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

	UMeleeAttackSystem* CombatComp = Character->GetMeleeAttackSystem();
	if (!CombatComp)
		return;

	// 무기 Sweep 종료
	CombatComp->StopWeaponSweep();

	UE_LOG(LogTemp, Log, TEXT("[ANS_WeaponTrace] Weapon sweep stopped for %s"), *Character->GetName());
}