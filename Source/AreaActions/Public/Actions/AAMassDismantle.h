// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AAAction.h"
#include "FGCrate.h"
#include "AAMassDismantle.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class AREAACTIONS_API AAAMassDismantle : public AAAAction
{
	GENERATED_BODY()

public:
	void Run() override;

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFGCrate> CrateClass;

	UPROPERTY(EditDefaultsOnly)
	float CrateDistance = 200.0;
};
