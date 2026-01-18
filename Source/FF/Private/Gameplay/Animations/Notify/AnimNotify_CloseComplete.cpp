// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Animations/Notify/AnimNotify_CloseComplete.h"
#include "Gameplay/Characters/Player_Base.h"
#include "Gameplay/Characters/PC_Base.h"

void UAnimNotify_CloseComplete::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (APlayer_Base* Owner = Cast<APlayer_Base>(MeshComp->GetOwner()))
	{
		if (APC_Base* PC_Base = Cast<APC_Base>(Owner->Controller))
		{
			PC_Base->CloseInventory();
		}
	}
}
