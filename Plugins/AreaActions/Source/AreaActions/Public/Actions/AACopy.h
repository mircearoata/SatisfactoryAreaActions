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
	virtual void OnCancel_Implementation() override;

	UFUNCTION(BlueprintCallable)
	void SetDelta(const FVector InDeltaPosition, const FRotator InDeltaRotation, const FVector InRotationCenter, const bool InIsRotationCenterSet)
	{
		this->DeltaPosition = InDeltaPosition;
		this->DeltaRotation = InDeltaRotation;
		this->RotationCenter = InRotationCenter;
		this->IsRotationCenterSet = InIsRotationCenterSet;
	}
	
	UFUNCTION(BlueprintPure)
	void GetDelta(FVector& OutDeltaPosition, FRotator& OutDeltaRotation, FVector& OutRotationCenter, bool& OutIsRotationCenterSet) const
	{
		OutDeltaPosition = this->DeltaPosition;
		OutDeltaRotation = this->DeltaRotation;
		OutRotationCenter = this->RotationCenter;
		OutIsRotationCenterSet = this->IsRotationCenterSet;
	}
	
	UFUNCTION(BlueprintImplementableEvent)
    void ShowCopyWidget();
	
	UFUNCTION(BlueprintCallable)
	void Preview();
	
	UFUNCTION(BlueprintCallable)
	void Finish();

	UFUNCTION()
    void RemoveMissingItemsWidget();
private:
	UPROPERTY()
	UAACopyBuildingsComponent* CopyBuildingsComponent;

	FVector DeltaPosition;
	FRotator DeltaRotation;
	FVector RotationCenter;
	bool IsRotationCenterSet;

	bool PreviewExists;

	UPROPERTY()
	UWidget* MissingItemsWidget;
};
