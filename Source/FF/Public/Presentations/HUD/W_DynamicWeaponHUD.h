// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "W_DynamicWeaponHUD.generated.h"

class APlayer_Base;
/**
 * BlueprintImplementableEvent 함수 선언 시 주의사항:
 * 1. FString 타입의 매개변수는 반드시 const reference로 선언해야 함 (const FString&)
 * 2. 그렇지 않으면 UHT(Unreal Header Tool)가 함수 시그니처를 제대로 인식하지 못해
 *    "overloaded member function not found" 컴파일 에러가 발생함
 */
UCLASS()
class FF_API UW_DynamicWeaponHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Weapon", meta = (DisplayName = "SetWeaponData"))
	void SetWeaponData(UTexture2D* Texture, const FString& WeaponName, int32 MaxAmmo, int32 CurrentClip);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Weapon", meta = (DisplayName = "UpdateAmmoCount"))
	void UpdateAmmoCount(int32 MaxAmmo, int32 CurrentClip);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Weapon", meta = (DisplayName = "ParseAmmoCount"))
	FString ParseAmmoCount(int32 AmmoCount);

public:
	UPROPERTY(BlueprintReadWrite, Category = "Player")
	APlayer_Base* CharacterRef;
};
