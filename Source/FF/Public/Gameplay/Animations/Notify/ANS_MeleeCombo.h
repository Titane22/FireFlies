// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_MeleeCombo.generated.h"

/**
 * AnimNotifyState for weapon sweep tracing during attack animations
 * - NotifyBegin: Starts weapon sweep (initializes trace settings)
 * - NotifyTick: Performs sphere trace along weapon blade
 * - NotifyEnd: Stops weapon sweep (clears hit actors)
 */
UCLASS()
class FF_API UANS_WeaponTrace : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UANS_WeaponTrace();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	/** Object types to detect for weapon sweep overlap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trace")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;

	/** Draw debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trace|Debug")
	bool bDrawDebug = true;

	/** Debug draw duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trace|Debug", meta = (EditCondition = "bDrawDebug"))
	float DebugDrawTime = 2.0f;

	/** Debug line thickness */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Trace|Debug", meta = (EditCondition = "bDrawDebug"))
	float DebugLineThickness = 3.0f;
};