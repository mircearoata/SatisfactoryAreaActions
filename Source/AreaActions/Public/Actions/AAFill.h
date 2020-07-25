// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AAAction.h"
#include "AACopyBuildingsComponent.h"
#include "AAFill.generated.h"

/**
* 
*/
UCLASS(Abstract, Blueprintable)
class AREAACTIONS_API AAAFill : public AAAAction
{
    GENERATED_BODY()

    public:
    AAAFill();
    void Run_Implementation() override;

    private:
    UPROPERTY()
    UAACopyBuildingsComponent* CopyBuildingsComponent;
};
