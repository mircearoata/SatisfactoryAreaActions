// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AASubsystemManager.h"
#include "Module/GameWorldModule.h"
#include "Util/SemVersion.h"

#include "AAGameWorldModule.generated.h"

/**
 * 
 */
UCLASS()
class AREAACTIONS_API UAAGameWorldModule : public UGameWorldModule
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, DisplayName = "GetAAInitMod", Meta = (DefaultToSelf = "WorldContextObject"))
	static UAAGameWorldModule* Get(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable)
	void InitSubsystemManager();

	FORCEINLINE AAASubsystemManager* GetSubsystemManager() const { return SubsystemManager; }

	UFUNCTION(BlueprintPure)
	static void GetAreaActionsVersion(FVersion& Version);

	virtual void DispatchLifecycleEvent(ELifecyclePhase Phase) override;

private:
	UPROPERTY()
	AAASubsystemManager* SubsystemManager;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AAASubsystemManager> SubsystemManagerClass;
};
