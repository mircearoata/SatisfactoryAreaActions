// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AAAction.h"
#include "FGRecipe.h"
#include "AASetRecipe.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class AREAACTIONS_API AAASetRecipe : public AAAAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetRecipe(TSubclassOf<UFGRecipe> SelectedRecipe, TMap<TSubclassOf<class AFGBuildableManufacturer>, int>& Statistics);
};
