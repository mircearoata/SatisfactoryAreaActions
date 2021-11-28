// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AAAction.h"
#include "AASetStationMode.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class AREAACTIONS_API AAASetStationMode : public AAAAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetStationMode(bool IsLoadMode);
};
