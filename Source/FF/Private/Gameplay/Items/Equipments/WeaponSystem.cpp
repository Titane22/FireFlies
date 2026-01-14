// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/Equipments/WeaponSystem.h"
#include "Gameplay/Items/Equipments/MasterWeapon.h"
#include "Gameplay/Characters/FFCharacter.h"
#include "Gameplay/Data/WeaponData.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Perception/AISense_Hearing.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

// Sets default values for this component's properties
UWeaponSystem::UWeaponSystem()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UWeaponSystem::FireCheck(int32 AmmoCount)
{
	if(Weapon_Details.Weapon_Data.CurrentAmmo == 0)
		return false;
	Weapon_Details.Weapon_Data.CurrentAmmo = Weapon_Details.Weapon_Data.CurrentAmmo - AmmoCount;
	return true;
}

void UWeaponSystem::FireFX(USoundBase* Sound, FVector Location, USoundAttenuation* AttenuationSettings, USoundConcurrency* ConcurrencySettings)
{
	if (!Sound)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::FireFX Sound is null"));
		return;
	}

	UGameplayStatics::SpawnSoundAtLocation(
		this,           // World context object
		Sound,          // Sound to play
		Location,       // Location to play sound at
		FRotator::ZeroRotator,  // Rotation (default to zero)
		1.0f,          // Volume multiplier
		1.0f,          // Pitch multiplier
		0.0f,          // Start time
		AttenuationSettings,    // Attenuation settings
		ConcurrencySettings     // Concurrency settings
	);

	UAISense_Hearing::ReportNoiseEvent(
		GetWorld(),
		Location,
		1.f,
		CharacterRef
	);
}

void UWeaponSystem::EmptyFX(USoundBase* Sound)
{
	UGameplayStatics::PlaySoundAtLocation(
		this,
		Sound,
		CharacterRef->GetActorLocation()
	);
}

void UWeaponSystem::MuzzleVFX(UNiagaraSystem* SystemTemplate, USceneComponent* AttachToComponent)
{
	if (!SystemTemplate || !AttachToComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("MuzzleVFX: SystemTemplate or AttachToComponent is null"));
		return;
	}

	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		SystemTemplate,           // 나이아가라 시스템 템플릿
		AttachToComponent,       // 부착할 컴포넌트
		NAME_None,               // 소켓 이름 (None으로 설정)
		FVector(0, 0, 0),        // 위치 오프셋
		FRotator(0, 0, 0),       // 회전 오프셋
		EAttachLocation::KeepRelativeOffset,  // 위치 타입
		true,                    // Auto Activate
		true,                    // Auto Destroy
		ENCPoolMethod::None,     // Pooling Method
		true                     // Pre Cull Check
	);

	if (!NiagaraComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("MuzzleVFX: Failed to spawn niagara component"));
	}
}

void UWeaponSystem::FireMontage(UAnimMontage* FireAnim)
{
	if (!CharacterRef)
		return;

	UAnimInstance* AnimInst = CharacterRef->GetMesh()->GetAnimInstance();
	if (!AnimInst)
		return;
	
	AnimInst->Montage_Play(FireAnim, 1.f);
}

bool UWeaponSystem::CheckAmmo()
{
	bool bHasAmmo = Weapon_Details.Weapon_Data.MaxAmmo > 0;
	bool bCanReload = Weapon_Details.Weapon_Data.CurrentAmmo < Weapon_Details.Weapon_Data.ClipAmmo;
	return bHasAmmo && bCanReload;
}

float UWeaponSystem::ReloadMontage(UAnimMontage* ReloadAnim)
{
	if (!CharacterRef || !ReloadAnim)
		return 0.0f;

	UAnimInstance* AnimInst = CharacterRef->GetMesh()->GetAnimInstance();
	if (!AnimInst)
		return 0.0f;

	return AnimInst->Montage_Play(ReloadAnim, 1.f);
}

void UWeaponSystem::ReloadCheck()
{
	Weapon_Details.Weapon_Data;
	if (Weapon_Details.Weapon_Data.MaxAmmo > 0)
	{
		if (Weapon_Details.Weapon_Data.CurrentAmmo == 0)
		{
			if (Weapon_Details.Weapon_Data.MaxAmmo >= Weapon_Details.Weapon_Data.ClipAmmo)
			{
				Weapon_Details.Weapon_Data.CurrentAmmo = Weapon_Details.Weapon_Data.ClipAmmo;
				Weapon_Details.Weapon_Data.MaxAmmo = Weapon_Details.Weapon_Data.MaxAmmo - Weapon_Details.Weapon_Data.ClipAmmo;
			}
			else
			{
				Weapon_Details.Weapon_Data.CurrentAmmo = Weapon_Details.Weapon_Data.MaxAmmo;
				Weapon_Details.Weapon_Data.MaxAmmo = 0;
			}
		}
		else
		{
			if (Weapon_Details.Weapon_Data.MaxAmmo >= Weapon_Details.Weapon_Data.ClipAmmo)
			{
				Weapon_Details.Weapon_Data.MaxAmmo = Weapon_Details.Weapon_Data.MaxAmmo - (Weapon_Details.Weapon_Data.ClipAmmo - Weapon_Details.Weapon_Data.CurrentAmmo);
				Weapon_Details.Weapon_Data.CurrentAmmo = Weapon_Details.Weapon_Data.ClipAmmo;
			}
			else
			{
				int32 RemainAmmo = Weapon_Details.Weapon_Data.ClipAmmo - Weapon_Details.Weapon_Data.CurrentAmmo;
				if (RemainAmmo > Weapon_Details.Weapon_Data.MaxAmmo)
				{
					Weapon_Details.Weapon_Data.CurrentAmmo = Weapon_Details.Weapon_Data.CurrentAmmo + Weapon_Details.Weapon_Data.MaxAmmo;
					Weapon_Details.Weapon_Data.MaxAmmo = 0;
				}
				else
				{
					Weapon_Details.Weapon_Data.CurrentAmmo = Weapon_Details.Weapon_Data.CurrentAmmo + RemainAmmo;
					Weapon_Details.Weapon_Data.MaxAmmo = Weapon_Details.Weapon_Data.MaxAmmo - RemainAmmo;
				}
			}
		}
		FireCheck(0);
	}
}
