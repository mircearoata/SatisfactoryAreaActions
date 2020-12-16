// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AASubsystem.h"
#include "FGSaveInterface.h"
#include "AAMigrationSubsystem.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class AREAACTIONS_API AAAMigrationSubsystem : public AAASubsystem, public IFGSaveInterface
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintPure, DisplayName = "GetAAMigrationSubsystem", Meta = (DefaultToSelf = "WorldContextObject"))
	static AAAMigrationSubsystem* Get(UObject* WorldContextObject);

	virtual bool ShouldSave_Implementation() const override;

public:
	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool bFixedOldCopyBuildings = false;
};
