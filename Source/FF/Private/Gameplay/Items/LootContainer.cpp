// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/LootContainer.h"
#include "Gameplay/Items/InventorySystem.h"
#include "Gameplay/Items/LootingSystem.h"

// Sets default values
ALootContainer::ALootContainer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("BaseMesh"));
	BaseMesh->SetupAttachment(RootComponent);

	InventorySystem = CreateDefaultSubobject<UInventorySystem>("InventorySystem");

	LootingSystem = CreateDefaultSubobject<ULootingSystem>("LootingSystem");

	LootingSystem->SetInventorySystem(InventorySystem);
}


bool ALootContainer::CanInteract_Implementation(AController* InstigatorRef) const
{
	if (!Super::CanInteract_Implementation(InstigatorRef))
		return false;

	return LootingSystem && LootingSystem->CanLooting();
}

// Called when the game starts or when spawned
void ALootContainer::BeginPlay()
{
	Super::BeginPlay();
	OnInteractionExecuted.AddDynamic(this, &ALootContainer::OnLootOpened);

	// InventorySystem 초기화 보장 (컴포넌트 BeginPlay 순서 이슈 방지)
	if (InventorySystem)
	{
		InventorySystem->InitializeGrid();
	}

	// LootingSystem의 GenerateLoot 호출 (bSpawnLoot가 true일 때)
	if (LootingSystem && LootingSystem->CanLooting())
	{
		LootingSystem->GenerateLoot();
	}
}

void ALootContainer::OnLootOpened(AInteraction* Interaction, const FInteractionContext& Context)
{
	if (!LootingSystem || !LootingSystem->CanLooting())
	{
		UE_LOG(LogTemp, Warning, TEXT("ALootContainer - LootingSystem is nullptr or LootingSystem Can't active OnLootOpened"));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(Context.InstigatorRef);
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("ALootContainer - AShooterPlayerController is nullptr"));
		return;
	}

	OpenLootingUI(PC);
}

void ALootContainer::OpenLootingUI(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}

	// TODO: PC->InteractLooting(InventorySystem);
}
