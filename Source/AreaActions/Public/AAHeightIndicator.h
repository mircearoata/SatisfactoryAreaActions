// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AAIndicator.h"
#include "AAHeightIndicator.generated.h"

UENUM()
enum EAAHeightIndicatorType {
	Top,
	Bottom
};

/**
 * 
 */
UCLASS(Abstract)
class AREAACTIONS_API AAAHeightIndicator : public AAAIndicator
{
	GENERATED_BODY()

public:
	FORCEINLINE void SetIndicatorType(const EAAHeightIndicatorType Type) { this->IndicatorType = Type; }

	virtual void UpdateHeight(float MinHeight, float MaxHeight) override;

private:
	EAAHeightIndicatorType IndicatorType;
};
