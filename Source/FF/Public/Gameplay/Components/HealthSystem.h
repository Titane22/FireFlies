// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

class AFFCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FF_API UHealthSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthSystem();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Health")
	float MaxHealth = 500.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Health")
	float CurrentHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Health")
	bool StartWithMaxHealth;

public:
	UPROPERTY()
	AFFCharacter* CharacterRef;

public:
	UFUNCTION(BlueprintCallable, Category = "Health")
	bool IsDead() const;
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	bool ApplyDamage(float Damage);

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool Heal(float HealAmount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetCurrentHealth();

	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetCurrentHealth(float Value);

	UFUNCTION(BlueprintCallable, Category = "Health")
	float GetMaxHealth();

	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetMaxHealth(float Value);

	// BlueprintAssignable로 BP에서도 바인딩 가능
	UPROPERTY(BlueprintAssignable, Category="Health")
	FOnHealthChanged OnHealthChanged;

	// BlueprintAssignable로 BP에서도 바인딩 가능
	UPROPERTY(BlueprintAssignable, Category="Health")
	FOnDeath OnDeath;
};
