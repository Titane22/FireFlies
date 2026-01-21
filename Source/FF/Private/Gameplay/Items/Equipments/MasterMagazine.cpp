// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Items/Equipments/MasterMagazine.h"
#include "Gameplay/Data/MagazineData.h"

UMasterMagazine::UMasterMagazine()
{
}

int32 UMasterMagazine::ConsumeAmmo(int32 Amount)
{
	if (Amount <= 0 || CurrentAmmo <= 0)
		return 0;

	int32 Consumed = FMath::Min(Amount, CurrentAmmo);
	CurrentAmmo -= Consumed;
	return Consumed;
}

int32 UMasterMagazine::AddAmmo(int32 Amount)
{
	if (Amount <= 0 || !MagazineData)
		return 0;

	int32 SpaceAvailable = MagazineData->ClipSize - CurrentAmmo;
	int32 Added = FMath::Min(Amount, SpaceAvailable);
	CurrentAmmo += Added;
	return Added;
}

bool UMasterMagazine::IsFull() const
{
	if (!MagazineData)
		return true;

	return CurrentAmmo >= MagazineData->ClipSize;
}

int32 UMasterMagazine::GetClipSize() const
{
	if (!MagazineData)
		return 0;

	return MagazineData->ClipSize;
}

void UMasterMagazine::FillMagazine()
{
	if (MagazineData)
	{
		CurrentAmmo = MagazineData->ClipSize;
	}
}

void UMasterMagazine::InitializeFromData(UMagazineData* InMagazineData, int32 InitialAmmo)
{
	MagazineData = InMagazineData;

	if (MagazineData)
	{
		SetInitialAmmo(InitialAmmo);
	}
}

void UMasterMagazine::SetInitialAmmo(int32 Amount)
{
	if (!MagazineData)
		return;

	if (Amount < 0)
	{
		// -1이면 가득 채움
		FillMagazine();
	}
	else
	{
		CurrentAmmo = FMath::Clamp(Amount, 0, MagazineData->ClipSize);
	}
}

/* TODO: 탄창 인스턴스 
// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Items/Equipments/MasterMagazine.h"
#include "Gameplay/Data/MagazineData.h"

AMasterMagazine::AMasterMagazine()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMasterMagazine::BeginPlay()
{
	Super::BeginPlay();

	// MagazineData가 설정되어 있으면 초기화
	if (MagazineData && CurrentAmmo == 0)
	{
		FillMagazine();
	}
}

int32 AMasterMagazine::ConsumeAmmo(int32 Amount)
{
	if (Amount <= 0 || CurrentAmmo <= 0)
		return 0;

	int32 Consumed = FMath::Min(Amount, CurrentAmmo);
	CurrentAmmo -= Consumed;
	return Consumed;
}

int32 AMasterMagazine::AddAmmo(int32 Amount)
{
	if (Amount <= 0 || !MagazineData)
		return 0;

	int32 SpaceAvailable = MagazineData->ClipSize - CurrentAmmo;
	int32 Added = FMath::Min(Amount, SpaceAvailable);
	CurrentAmmo += Added;
	return Added;
}

bool AMasterMagazine::IsFull() const
{
	if (!MagazineData)
		return true;

	return CurrentAmmo >= MagazineData->ClipSize;
}

int32 AMasterMagazine::GetClipSize() const
{
	if (!MagazineData)
		return 0;

	return MagazineData->ClipSize;
}

void AMasterMagazine::FillMagazine()
{
	if (MagazineData)
	{
		CurrentAmmo = MagazineData->ClipSize;
	}
}

void AMasterMagazine::InitializeFromData(UMagazineData* InMagazineData, int32 InitialAmmo)
{
	MagazineData = InMagazineData;

	if (MagazineData)
	{
		SetInitialAmmo(InitialAmmo);
	}
}

void AMasterMagazine::SetInitialAmmo(int32 Amount)
{
	if (!MagazineData)
		return;

	if (Amount < 0)
	{
		// -1이면 가득 채움
		FillMagazine();
	}
	else
	{
		CurrentAmmo = FMath::Clamp(Amount, 0, MagazineData->ClipSize);
	}
}
*/