// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Components/WeaponSystem.h"
#include "Gameplay/Equipments/Weapons/Weapon_Base.h"
#include "Gameplay/Characters/FFCharacter.h"
#include "Gameplay/Data/WeaponData.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UWeaponSystem::UWeaponSystem()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UWeaponSystem::SetWeaponState(TSubclassOf<AWeapon_Base> ToSetWeaponClass, EAnimationState ToSetAnimation,
	EWeaponState CurWeaponState, FName ToSetEquipSocketName, FName ToSetUnequipSocketName, EWeaponSlot WeaponSlot)
{
	FWeapon_Details CurrentDetails;

	// WeaponSlot에 따라 적절한 ChildActorComponent 선택
	UChildActorComponent* TargetChild = (WeaponSlot == EWeaponSlot::Primary)
		? CharacterRef->PrimaryChild
		: CharacterRef->HandgunChild;

	if (TargetChild)
	{
		if (AWeapon_Base* CurrentWeapon = Cast<AWeapon_Base>(TargetChild->GetChildActor()))
		{
			UWeaponSystem* CurrentWS = CurrentWeapon->GetWeaponSystem();
			if (CurrentWS)
			{
				CurrentDetails = CurrentWS->Weapon_Details;
			}
		}
	}

	MasterWeapon = ToSetWeaponClass;
	AnimationState = ToSetAnimation;

	switch (CurWeaponState)
	{
	case EWeaponState::Equip:
		EquipWeapon(ToSetEquipSocketName, WeaponSlot);
		break;
	case EWeaponState::Unequip:
		UnequipWeapon(ToSetUnequipSocketName, WeaponSlot);
		break;
	}

	if (TargetChild)
	{
		if (AWeapon_Base* NewWeapon = Cast<AWeapon_Base>(TargetChild->GetChildActor()))
		{
			UWeaponSystem* CurrentWS = NewWeapon->GetWeaponSystem();
			if (CurrentWS)
			{
				CurrentWS->Weapon_Details = CurrentDetails;
			}
		}
	}
}

void UWeaponSystem::EquipWeapon(FName SocketName, EWeaponSlot WeaponSlot)
{
	if (!CharacterRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::EquipWeapon CharacterRef is Null"));
		return;
	}

	// WeaponSlot에 따라 적절한 ChildActorComponent 선택
	UChildActorComponent* TargetChild = (WeaponSlot == EWeaponSlot::Primary)
		? CharacterRef->PrimaryChild
		: CharacterRef->HandgunChild;

	if (!TargetChild)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::EquipWeapon TargetChild is Null"));
		return;
	}

	// Child Actor Class 설정
	TargetChild->SetChildActorClass(MasterWeapon);

	// WeaponSystem 참조 설정
	UWeaponSystem* TargetWeaponSystem = Cast<UWeaponSystem>(
		TargetChild->GetChildActor()->GetComponentByClass(this->GetClass())
	);
	if (!TargetWeaponSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::EquipWeapon TargetWeaponSystem is Null"));
		return;
	}

	TargetWeaponSystem->CharacterRef = CharacterRef;

	// 소켓에 부착
	TargetChild->AttachToComponent(
		CharacterRef->GetMesh(),
		FAttachmentTransformRules(
			EAttachmentRule::SnapToTarget,    // Location Rule
			EAttachmentRule::SnapToTarget,    // Rotation Rule
			EAttachmentRule::SnapToTarget,    // Scale Rule
			true                              // Weld Simulated Bodies
		),
		SocketName
	);

	CharacterRef->CurrentAnimationState = AnimationState;
}

void UWeaponSystem::UnequipWeapon(FName SocketName, EWeaponSlot WeaponSlot)
{
	if (!CharacterRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::UnequipWeapon CharacterRef is Null"));
		return;
	}

	// WeaponSlot에 따라 적절한 ChildActorComponent 선택
	UChildActorComponent* TargetChild = (WeaponSlot == EWeaponSlot::Primary)
		? CharacterRef->PrimaryChild
		: CharacterRef->HandgunChild;

	if (!TargetChild)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponSystem::UnequipWeapon TargetChild is Null"));
		return;
	}

	// Child Actor Class 설정
	TargetChild->SetChildActorClass(MasterWeapon);

	// 소켓에 부착
	TargetChild->AttachToComponent(
		CharacterRef->GetMesh(),
		FAttachmentTransformRules(
			EAttachmentRule::SnapToTarget,    // Location Rule
			EAttachmentRule::SnapToTarget,    // Rotation Rule
			EAttachmentRule::SnapToTarget,    // Scale Rule
			true                              // Weld Simulated Bodies
		),
		SocketName
	);

	CharacterRef->CurrentAnimationState = AnimationState;
}
 
bool UWeaponSystem::FireCheck(int32 AmmoCount)
{
	if (Weapon_Details.Weapon_Data.CurrentAmmo == 0)
		return false;
	Weapon_Details.Weapon_Data.CurrentAmmo = Weapon_Details.Weapon_Data.CurrentAmmo - AmmoCount;
	return true;
}

void UWeaponSystem::FireFX(USoundBase* Sound, FVector Location, USoundAttenuation* AttenuationSettings,
	USoundConcurrency* Concurrency)
{
	if (!Sound)
		return;

	UGameplayStatics::SpawnSoundAtLocation(
		this,           // World context object
		Sound,          // Sound to play
		Location,       // Location to play sound at
		FRotator::ZeroRotator,  // Rotation (default to zero)
		1.0f,          // Volume multiplier
		1.0f,          // Pitch multiplier
		0.0f,          // Start time
		AttenuationSettings,    // Attenuation settings
		Concurrency     // Concurrency settings
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

void UWeaponSystem::MuzzleVFX(UNiagaraSystem* NiagaraSystem, USceneComponent* AttachToComponent)
{
	if (!NiagaraSystem || !AttachToComponent)
	{
		return;
	}

	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		NiagaraSystem,           // 나이아가라 시스템 템플릿
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
		return;
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
	// 예비 탄약이 없으면 리로드 불가
	if (Weapon_Details.Weapon_Data.MaxAmmo <= 0)
		return;

	// 필요한 탄약 계산
	int32 AmmoNeeded = Weapon_Details.Weapon_Data.ClipAmmo - Weapon_Details.Weapon_Data.CurrentAmmo;
	int32 AmmoToReload = FMath::Min(AmmoNeeded, Weapon_Details.Weapon_Data.MaxAmmo);

	// 탄약 업데이트
	Weapon_Details.Weapon_Data.CurrentAmmo += AmmoToReload;
	Weapon_Details.Weapon_Data.MaxAmmo -= AmmoToReload;
}


// Called when the game starts
void UWeaponSystem::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}
