// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/Equipments/EquipmentBase.h"

#include "Misc/MapErrors.h"

// Sets default values
AEquipmentBase::AEquipmentBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DefaultRoot = CreateDefaultSubobject<USceneComponent>("DefaultRoot");
	EquipmentMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Equipment Skeletal Mesh"));
	EquipmentStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("Equipment Static Mesh");
	InteractCollision = CreateDefaultSubobject<USphereComponent>(TEXT("InteractCollision"));

	// Setting up the component hierarchy
	RootComponent = DefaultRoot;
	EquipmentMesh->SetupAttachment(RootComponent);
	EquipmentMesh->SetCollisionProfileName(FName("PhysicsActor"));

	EquipmentStaticMesh->SetupAttachment(RootComponent);
	EquipmentStaticMesh->SetCollisionProfileName(FName("PhysicsActor"));
	
	InteractCollision->SetupAttachment(EquipmentMesh);
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

