// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Characters/PC_Base.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/Characters/Player_Base.h"

bool APC_Base::IsVisibleWidget() const
{
	if (!InvenWidget)
		return false;

	return InvenWidget->IsVisible();
}

void APC_Base::SetIgnoreInput(bool bIgnoreInput)
{
	SetIgnoreLookInput(bIgnoreInput);
	SetIgnoreMoveInput(bIgnoreInput);
}

void APC_Base::BeginPlay()
{
	Super::BeginPlay();

	OwnerRef = Cast<APlayer_Base>(GetPawn());
}

void APC_Base::AddInputModeTag(FGameplayTag Tag)
{
	InputModeTags.AddTag(Tag);
}

void APC_Base::RemoveInputModeTag(FGameplayTag Tag)
{
	InputModeTags.RemoveTag(Tag);
}

bool APC_Base::HasInputModeTag(FGameplayTag Tag) const
{
	return InputModeTags.HasTag(Tag);
}
