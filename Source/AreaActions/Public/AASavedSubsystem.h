// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AASubsystem.h"
#include "FGSaveInterface.h"
#include "AASavedSubsystem.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class AREAACTIONS_API AAASavedSubsystem : public AAASubsystem, public IFGSaveInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, DisplayName = "GetAASavedSubsystem", Meta = (DefaultToSelf = "WorldContextObject"))
	static AAASavedSubsystem* Get(UObject* WorldContextObject);

	virtual bool ShouldSave_Implementation() const override;

public:
	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool mIsFlying = false;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	float mMaxContainerDistance = 50000.0;
};
