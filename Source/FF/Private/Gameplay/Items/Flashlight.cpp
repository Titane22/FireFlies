// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Items/Flashlight.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
AFlashlight::AFlashlight()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("FlashLightArm"));
	RootComponent = SpringArm;

	FlashMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FlashMesh"));
	FlashMesh->SetupAttachment(RootComponent);

	SpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight"));
	SpotLight->SetupAttachment(FlashMesh);
	
	SpringArm->TargetArmLength = 200.f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 12.f;
}

// Called when the game starts or when spawned
void AFlashlight::BeginPlay()
{
	Super::BeginPlay();

	SpotLight->AttenuationRadius = 3500.f;
	SpotLight->Intensity = 8000.f;
	SpotLight->bUseInverseSquaredFalloff = true;
	SpotLight->OuterConeAngle = 35.f;
	SpotLight->InnerConeAngle = 15.f;
}

// Called every frame
void AFlashlight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

