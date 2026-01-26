// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Characters/Enemy/Enemy_Base.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Gameplay/Characters/Enemy/AIC_Base.h"

AEnemy_Base::AEnemy_Base()
	:Super()
{
	// Set this character to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	// AI 컨트롤러 클래스 설정
	AIControllerClass = AAIC_Base::StaticClass();
	
	// 자동으로 AI 컨트롤러 Possess 설정
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemy_Base::BeginPlay()
{
	Super::BeginPlay();
	if (Primary)
	{
		Primary->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("riflehost_Socket"));
		TArray<USceneComponent*> AttachedChildren = Primary->GetAttachChildren();
		if (AttachedChildren.Num() > 0)
		{
			PrimaryChild = Cast<UChildActorComponent>(AttachedChildren[0]);
			if (PrimaryChild)
			{
				UE_LOG(LogTemp, Warning, TEXT("Successed PrimaryChild###2"));
			}
		}
	}
	if (Handgun)
	{
		Handgun->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("pistolhost_Socket"));
		TArray<USceneComponent*> AttachedChildren = Handgun->GetAttachChildren();
		if (AttachedChildren.Num() > 0)
		{
			HandgunChild = Cast<UChildActorComponent>(AttachedChildren[0]);
			if (HandgunChild)
			{
				UE_LOG(LogTemp, Warning, TEXT("Successed HandgunChild###2"));
			}
		}
	}
}
