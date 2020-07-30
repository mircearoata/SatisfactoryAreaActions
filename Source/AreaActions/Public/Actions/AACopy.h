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
	virtual void Run_Implementation() override;

	UFUNCTION(BlueprintCallable)
	void SetDelta(const FVector InDeltaPosition, const FRotator InDeltaRotation, FVector InRotationCenter) { this->DeltaPosition = InDeltaPosition; this->DeltaRotation = InDeltaRotation; }
	
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
