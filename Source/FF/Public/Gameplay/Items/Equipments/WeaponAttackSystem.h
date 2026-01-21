// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponAttackSystem.generated.h"

class AFFCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FF_API UWeaponAttackSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponAttackSystem();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		
	// Hit 처리 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool ApplyHit(const FHitResult HitResult, float DamageAmount, bool& ValidHit);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void PerformAttack() PURE_VIRTUAL(UWeaponAttackSystem::PerformAttack);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual bool CanReload() const { return false; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void ExecuteReload() {}

	// UI 업데이트를 위한 탄약 정보 getter
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual int32 GetCurrentAmmo() const { return 0; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual int32 GetMaxAmmo() const { return 0; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void SetCurrentAmmo(float Amount) { }

public:
	UPROPERTY()
	AFFCharacter* CharacterRef = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bReloading = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bIsDryAmmo = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bAutoReload = false;
};
