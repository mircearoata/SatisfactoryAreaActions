// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AAIndicator.h"
#include "AAHeightIndicator.generated.h"

UENUM()
enum EAAHeightIndicatorType {
	TOP,
	BOTTOM
};

/**
 * 
 */
UCLASS(Abstract)
class AREAACTIONS_API AAAHeightIndicator : public AAAIndicator
{
	GENERATED_BODY()

public:
	FORCEINLINE void SetIndicatorType(EAAHeightIndicatorType type) { this->mIndicatorType = type; }

	void UpdateHeight(float minHeight, float maxHeight) override;

private:
	EAAHeightIndicatorType mIndicatorType;
};
