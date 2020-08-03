// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AASubsystem.h"
#include "AAAction.h"
#include "AAActionsManager.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class AREAACTIONS_API AAAActionsManager : public AAASubsystem
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, DisplayName = "GetAAActionsManager", Meta = (DefaultToSelf = "WorldContextObject"))
	static AAAActionsManager* Get(UObject* WorldContextObject);

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, SaveGame)
	TArray<TSubclassOf<AAAAction>> AvailableActions;
};
