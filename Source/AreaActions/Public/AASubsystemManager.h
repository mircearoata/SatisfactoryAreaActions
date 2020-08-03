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
	virtual void BeginPlay() override;

	virtual bool ShouldSave_Implementation() const override;
	virtual void GatherDependencies_Implementation(TArray< UObject* >& OutDependentObjects) override;

	UFUNCTION(BlueprintPure, DisplayName = "GetAASubsystemManager", Meta = (DefaultToSelf = "WorldContextObject"))
	static AAASubsystemManager* Get(UObject* WorldContextObject);

	FORCEINLINE AAASavedSubsystem* GetSavedSubsystem() const { return SavedSubsystem; }
	FORCEINLINE AAAActionsManager* GetActionsManager() const { return ActionsManager; }

private:
	UPROPERTY(SaveGame)
	AAASavedSubsystem* SavedSubsystem;

	UPROPERTY(SaveGame)
	AAAActionsManager* ActionsManager;

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AAASavedSubsystem> SavedSubsystemClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AAAActionsManager> ActionsManagerClass;
};
