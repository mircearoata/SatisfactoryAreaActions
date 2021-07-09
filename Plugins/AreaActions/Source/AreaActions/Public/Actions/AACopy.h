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
	void SetDelta(const FVector InDeltaPosition, const FRotator InDeltaRotation)
	{
		this->DeltaPosition = InDeltaPosition;
		this->DeltaRotation = InDeltaRotation;
	}

	UFUNCTION(BlueprintCallable)
	void SetAnchor(AFGBuildable* InAnchor)
	{
		this->Anchor = InAnchor;
	}
	
	UFUNCTION(BlueprintPure)
	void GetDelta(FVector& OutDeltaPosition, FRotator& OutDeltaRotation) const
	{
		OutDeltaPosition = this->DeltaPosition;
		OutDeltaRotation = this->DeltaRotation;
	}
	
	UFUNCTION(BlueprintPure)
	void GetAnchor(AFGBuildable*& OutAnchor) const
	{
		OutAnchor = this->Anchor;
	}
	
	UFUNCTION(BlueprintImplementableEvent)
    void ShowCopyWidget();
	
	UFUNCTION(BlueprintCallable)
	void Preview();
	
	UFUNCTION(BlueprintCallable)
	void Finish();

	UFUNCTION()
	void RemoveMissingItemsWidget();
	
protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UAACopyBuildingsComponent* CopyBuildingsComponent;

	FVector DeltaPosition;
	FRotator DeltaRotation;
	
	UPROPERTY()
	AFGBuildable* Anchor;

	UPROPERTY()
	UWidget* MissingItemsWidget;
};
