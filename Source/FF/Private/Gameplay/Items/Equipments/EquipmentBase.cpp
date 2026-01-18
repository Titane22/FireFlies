// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/Equipments/EquipmentBase.h"

// Sets default values
AEquipmentBase::AEquipmentBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	EquipmentMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("EquipmentMesh"));
	InteractCollision = CreateDefaultSubobject<USphereComponent>(TEXT("InteractCollision"));

	// Setting up the component hierarchy
	RootComponent = EquipmentMesh;
	InteractCollision->SetupAttachment(EquipmentMesh);

	// Set InteractBox collision preset to Interactable
	InteractCollision->SetCollisionProfileName(FName("Interactable"));
}

// Called when the game starts or when spawned
void AEquipmentBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEquipmentBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

