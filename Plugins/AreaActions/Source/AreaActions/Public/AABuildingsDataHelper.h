// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AARotatedBoundingBox.h"
#include "Resources/FGItemDescriptor.h"

/**
 * 
 */
class AREAACTIONS_API FAABuildingsDataHelper
{
public:
	static FAARotatedBoundingBox CalculateBoundingBox(const TArray<UObject*>& Objects);
	static TMap<TSubclassOf<UFGItemDescriptor>, int32> CalculateBuildCosts(const TArray<UObject*>& Objects);
	static TMap<TSubclassOf<UFGItemDescriptor>, int32> CalculateOtherItems(const TArray<UObject*>& Objects);
};
