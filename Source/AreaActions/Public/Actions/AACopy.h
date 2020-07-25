// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AAAction.h"
#include "AACopyBuildingsComponent.h"
#include "AACopy.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class AREAACTIONS_API AAACopy : public AAAAction
{
	GENERATED_BODY()

public:
	AAACopy();
	void Run_Implementation() override;

	UFUNCTION(BlueprintCallable)
	void SetDelta(FVector DeltaPosition, FRotator DeltaRotation, FVector RotationCenter) { this->DeltaPosition = DeltaPosition; this->DeltaRotation = DeltaRotation; }
	
	UFUNCTION(BlueprintCallable)
	void Preview();
	
	UFUNCTION(BlueprintCallable)
	void Finish();
private:
	UPROPERTY()
	UAACopyBuildingsComponent* CopyBuildingsComponent;

	FVector DeltaPosition;
	FRotator DeltaRotation;

	bool PreviewExists;
};
