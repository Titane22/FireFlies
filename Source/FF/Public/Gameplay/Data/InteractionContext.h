// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Data/InteractionData.h"
#include "InteractionContext.generated.h"

class AInteraction;

/**
 * Interaction Context - 상호작용 시 필요한 모든 컨텍스트 정보를 담는 구조체
 * 이 구조체를 통해 상호작용 시스템 전반에 걸쳐 일관된 데이터 전달
 */
USTRUCT(BlueprintType)
struct FInteractionContext
{
	GENERATED_BODY()

	/** 상호작용을 요청한 컨트롤러 */
	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	AController* InstigatorRef = nullptr;

	/** 상호작용을 요청한 폰 */
	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	APawn* InstigatorPawn = nullptr;

	/** 상호작용 대상 액터 */
	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	AInteraction* TargetActor = nullptr;

	/** 상호작용 데이터 */
	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	UInteractionData* InteractionData = nullptr;

	/** 상호작용 거리 */
	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	float Distance = 0.0f;

	/** 상호작용 시작 시간 (Hold 타입용) */
	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	float StartTime = 0.0f;

	/** 추가 컨텍스트 데이터 (확장 가능) */
	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	TMap<FName, FString> AdditionalData;

	/** 기본 생성자 */
	FInteractionContext()
		: InstigatorRef(nullptr)
		, InstigatorPawn(nullptr)
		, TargetActor(nullptr)
		, InteractionData(nullptr)
		, Distance(0.0f)
		, StartTime(0.0f)
	{
	}

	/** 완전한 생성자 */
	FInteractionContext(AController* InInstigator, AInteraction* InTarget, UInteractionData* InData, float InDistance = 0.0f)
		: InstigatorRef(InInstigator)
		, InstigatorPawn(InInstigator ? InInstigator->GetPawn() : nullptr)
		, TargetActor(InTarget)
		, InteractionData(InData)
		, Distance(InDistance)
		, StartTime(0.0f)
	{
	}

	/** 유효성 체크 */
	bool IsValid() const
	{
		return InstigatorRef != nullptr && TargetActor != nullptr && InteractionData != nullptr;
	}

	/** 거리 체크가 필요한지 확인 */
	bool ShouldCheckDistance() const
	{
		return InteractionData && InteractionData->bCheckDistance;
	}

	/** 최대 거리 내에 있는지 확인 */
	bool IsWithinRange() const
	{
		if (!ShouldCheckDistance() || !InteractionData)
			return true;

		return Distance <= InteractionData->DetectionRadius;
	}
};

/**
 * Interaction Result - 상호작용 결과
 */
USTRUCT(BlueprintType)
struct FInteractionResult
{
	GENERATED_BODY()

	/** 상호작용 성공 여부 */
	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	bool bSuccess = false;

	/** 실패 이유 */
	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	FText FailureReason;

	/** 추가 결과 데이터 */
	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	TMap<FName, FString> ResultData;

	/** 기본 생성자 */
	FInteractionResult()
		: bSuccess(false)
	{
	}

	/** 성공 생성자 */
	static FInteractionResult Success()
	{
		FInteractionResult Result;
		Result.bSuccess = true;
		return Result;
	}

	/** 실패 생성자 */
	static FInteractionResult Failure(const FText& Reason)
	{
		FInteractionResult Result;
		Result.bSuccess = false;
		Result.FailureReason = Reason;
		return Result;
	}
};
