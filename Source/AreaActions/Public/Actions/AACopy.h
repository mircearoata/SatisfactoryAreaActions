// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AAAction.h"
#include "AACopyBuildingsComponent.h"
#include "AACopy.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class AREAACTIONS_API AAACopy : public AAAAction
{
	GENERATED_BODY()

public:
	AAACopy();
	void Run_Implementation() override;

private:
	UPROPERTY()
	UAACopyBuildingsComponent* mCopyBuildingsComponent;
};
