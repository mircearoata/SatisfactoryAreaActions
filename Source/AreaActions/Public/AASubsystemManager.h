// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "FGSaveInterface.h"
#include "AASavedSubsystem.h"
#include "AAActionsManager.h"
#include "AASubsystemManager.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class AREAACTIONS_API AAASubsystemManager : public AInfo, public IFGSaveInterface
{
	GENERATED_BODY()

public:
	void BeginPlay() override;

	virtual bool ShouldSave_Implementation() const override;
	virtual void GatherDependencies_Implementation(TArray< UObject* >& out_dependentObjects) override;

	UFUNCTION(BlueprintPure, DisplayName = "GetAASubsystemManager", Meta = (DefaultToSelf = "WorldContextObject"))
	static AAASubsystemManager* Get(UObject* WorldContextObject);

	FORCEINLINE AAASavedSubsystem* GetSavedSubsystem() { return mSavedSubsystem; }
	FORCEINLINE AAAActionsManager* GetActionsManager() { return mActionsManager; }

private:
	UPROPERTY(SaveGame)
	AAASavedSubsystem* mSavedSubsystem;

	UPROPERTY(SaveGame)
	AAAActionsManager* mActionsManager;

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AAASavedSubsystem> SavedSubsystemClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AAAActionsManager> ActionsManagerClass;
};
