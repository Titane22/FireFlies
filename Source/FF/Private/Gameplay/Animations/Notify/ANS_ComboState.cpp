// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Animations/Notify/ANS_ComboState.h"

#include "Gameplay/Characters/FFCharacter.h"
#include "Gameplay/Items/Equipments/MeleeAttackSystem.h"

void UANS_ComboState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
		return;

	AFFCharacter* Character = Cast<AFFCharacter>(MeshComp->GetOwner());
	if (!Character)
		return;

	UMeleeAttackSystem* CombatComp = Character->GetMeleeAttackSystem();
	if (!CombatComp)
		return;

	CombatComp->SetComboState(ComboType, true);
}

void UANS_ComboState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
		return;

	AFFCharacter* Character = Cast<AFFCharacter>(MeshComp->GetOwner());
	if (!Character)
		return;

	UMeleeAttackSystem* CombatComp = Character->GetMeleeAttackSystem();
	if (!CombatComp)
		return;

	CombatComp->SetComboState(ComboType, false);
}
