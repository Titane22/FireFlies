// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponAttackSystem.h"
#include "GunAttackSystem.generated.h"

class AFFCharacter;
class USkeletalMesh;
class AMasterWeapon;
class UWeaponData;
class UUserWidget;
class UNiagaraSystem;
class USceneComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FF_API UGunAttackSystem : public UWeaponAttackSystem
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGunAttackSystem();

	virtual void BeginPlay() override;

	virtual void PerformAttack() override;

	// 입력 라우팅 오버라이드
	virtual void OnAttackStarted() override;
	virtual void OnAttackHeld() override;
	virtual void OnAttackReleased() override;

	virtual bool CanReload() const override;
	virtual void ExecuteReload() override;
	virtual int32 GetCurrentAmmo() const override;
	virtual int32 GetMaxAmmo() const override;
	virtual void SetCurrentAmmo(float Amount) override;

	void FireBullet(FHitResult Hit, bool bReturnHit);

	bool FireCheck(int32 AmmoCount);

	void FireFX(USoundBase* Sound, FVector Location, USoundAttenuation* AttenuationSettings, USoundConcurrency* Concurrency);

	void EmptyFX(USoundBase* Sound);

	void MuzzleVFX(UNiagaraSystem* NiagaraSystem, USceneComponent* AttachToComponent);

	void FireMontage(UAnimMontage* FireAnim);

	bool CheckAmmo() const;

	float ReloadMontage(UAnimMontage* ReloadAnim);

	void ReloadCheck();

	void RandPointInCircle(float Radius, float& PointX, float& PointY);

	void FireBlankTracer();

	void PlayFireEffect();
private:
	// Fire helper functions
	void ApplyCameraShake(APlayerController* PC);
	bool PerformCameraTrace(APlayerCameraManager* CameraManager, FHitResult& OutHitResult);
	void ExecuteFireSequence(const FHitResult& CameraHitResult);

	// 스프레드가 적용된 타겟 포인트 계산 (Hitscan/Projectile 공통)
	FVector ApplySpreadToTarget(const FVector& BaseTarget, APlayerCameraManager* PCM);

	// Hitscan 전용: 스프레드 적용된 라인 트레이스 + 즉시 데미지
	void PerformHitscanFire();

	// Hitscan 전용: 스프레드 적용된 단일 라인 트레이스
	bool PerformSpreadTrace(FHitResult& OutHitResult, FVector& OutTraceEnd);

	// 시각 전용 궤적 스폰 (Hitscan에서 사용, 데미지 없음)
	void SpawnVisualTracer(const FVector& Start, const FVector& End);

	// 히트 마커 사운드 + HUD 피드백 재생
	void PlayHitFeedback(bool bIsKill);

	// 발사 상태 (Player_Base에서 이동)
	void HandleFiring();
	void ReadyToFire();
	void UpdateAmmoUI();

	bool bFiring = false;
	bool bCanFire = true;
	FTimerHandle FireRateTimerHandle;

public:
	/** 총알 궤적 Actor의 최대 생존 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|BulletTrace")
	float BulletTraceLifeSpan = 3.0f;

private:
	int32 BurstShotsFired = 0;
};
