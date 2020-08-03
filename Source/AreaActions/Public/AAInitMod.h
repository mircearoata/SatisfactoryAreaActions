// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "mod/actor/SMLInitMod.h"
#include "AASubsystemManager.h"
#include "AAInitMod.generated.h"

/**
 * 
 */
UCLASS()
class AREAACTIONS_API AAAInitMod : public ASMLInitMod
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, DisplayName = "GetAAInitMod", Meta = (DefaultToSelf = "WorldContextObject"))
	static AAAInitMod* Get(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable)
	void InitSubsystemManager();

	FORCEINLINE AAASubsystemManager* GetSubsystemManager() const { return SubsystemManager; }

private:
	UPROPERTY()
	AAASubsystemManager* SubsystemManager;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AAASubsystemManager> SubsystemManagerClass;
};
