// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Components/Hurtbox.h"
#include "Gameplay/Characters/FFCharacter.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"

float UHurtbox::GetDamageMultiplier(const FName HitBoneName) const
{
	// 기본 배율
	float Multiplier = 1.0f;

	// BoneName이 유효하고 DamageMultipliers에 등록되어 있으면 해당 배율 사용
	if (!HitBoneName.IsNone() && DamageMultipliers.Contains(HitBoneName))
	{
		Multiplier = DamageMultipliers[HitBoneName];
	}

	return Multiplier;
}

void UHurtbox::ApplyHitReaction(const FVector& HitLocation, const FVector& HitDirection, const FName BoneName, float Force)
{
	if (!CharacterRef)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterRef is NULL"));
		return;
	}

	UPhysicalAnimationComponent* PAC = CharacterRef->GetPAC();
	if (!PAC)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterRef's PhysicalAnimationComponent is NULL"));
		return;
	}

	USkeletalMeshComponent* Mesh = CharacterRef->GetMesh();
	if (!Mesh)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterRef's Mesh is NULL"));
		return;
	}
	
	UWorld* World = CharacterRef->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("World is NULL"));
		return;
	}

	// ✅ 단일 본만 물리 시뮬레이션 활성화
	FBodyInstance* BodyInstance = Mesh->GetBodyInstance(BoneName);
	if (!BodyInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("No BodyInstance found for bone: %s"), *BoneName.ToString());
		return;
	}

	// 해당 본만 물리 활성화 및 설정
	BodyInstance->SetInstanceSimulatePhysics(true);
	BodyInstance->SetEnableGravity(false);  // 중력 비활성화로 바닥에 쓰러지는 것 방지
	BodyInstance->PhysicsBlendWeight = 0.2f;  // 물리 영향 최소화 (0.5 → 0.2)

	// ✅ 단일 본에만 Physical Animation 적용 (강한 복원력)
	FPhysicalAnimationData PhysAnimData;
	PhysAnimData.bIsLocalSimulation = true;
	PhysAnimData.OrientationStrength = 10000.0f;      // 애니메이션 포즈로 강하게 복귀
	PhysAnimData.AngularVelocityStrength = 500.0f;   // 회전 속도 제어
	PhysAnimData.PositionStrength = 10000.0f;         // 위치 복원력
	PhysAnimData.VelocityStrength = 500.0f;          // 속도 제어
	PhysAnimData.MaxLinearForce = 10000.0f;          // 최대 선형 힘
	PhysAnimData.MaxAngularForce = 10000.0f;         // 최대 회전 힘

	PAC->ApplyPhysicalAnimationSettings(BoneName, PhysAnimData);

	// Z축 제거하여 공중으로 날아가는 것 방지
	FVector SafeDir = HitDirection.GetSafeNormal();
	SafeDir.Z = 0.0f;

	// 임펄스 힘 감소 (500 → 100)
	Mesh->AddImpulseAtLocation(SafeDir * (Force * 0.2f), HitLocation, BoneName);

	ActivatedBoneName = BoneName;

	World->GetTimerManager().SetTimer(
		RecoveryTimer,
		this,
		&UHurtbox::RecoverFromHit,
		0.5f
	);
}

void UHurtbox::RecoverFromHit()
{
	if (!CharacterRef)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterRef is NULL"));
		return;
	}

	UPhysicalAnimationComponent* PAC = CharacterRef->GetPAC();
	if (!PAC) return;

	USkeletalMeshComponent* Mesh = CharacterRef->GetMesh();
	if (!Mesh) return;

	// ✅ 해당 본만 물리 해제 및 블렌드 가중치 복구
	FBodyInstance* BodyInstance = Mesh->GetBodyInstance(ActivatedBoneName);
	if (BodyInstance)
	{
		BodyInstance->SetInstanceSimulatePhysics(false);
		BodyInstance->PhysicsBlendWeight = 0.0f;
	}

	// Physical Animation 제거
	FPhysicalAnimationData EmptyData;
	PAC->ApplyPhysicalAnimationSettings(ActivatedBoneName, EmptyData);

	ActivatedBoneName = NAME_None;
}
