// Fill out your copyright notice in the Description page of Project Settings.

#include "Gameplay/Data/InteractionData.h"

UInteractionData::UInteractionData()
{
	// 기본값 설정
	InteractionType = EInteractiveType::Default;
	InteractionName = FText::FromString("Interact");
	bIsEnabled = true;
	DetectionRadius = 200.0f;
	HoldDuration = 0.0f;
	bSingleUse = false;
	CooldownTime = 0.0f;
	PromptText = FText::FromString("Press E to interact");
	HighlightColor = FLinearColor::Green;
	bCheckDistance = true;
}

#if WITH_EDITOR
void UInteractionData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 에디터에서 값 변경 시 유효성 검증
	if (PropertyChangedEvent.Property)
	{
		const FName PropertyName = PropertyChangedEvent.Property->GetFName();

		// 음수 값 방지
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UInteractionData, DetectionRadius))
		{
			DetectionRadius = FMath::Max(0.0f, DetectionRadius);
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(UInteractionData, HoldDuration))
		{
			HoldDuration = FMath::Max(0.0f, HoldDuration);
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(UInteractionData, CooldownTime))
		{
			CooldownTime = FMath::Max(0.0f, CooldownTime);
		}
	}
}
#endif
