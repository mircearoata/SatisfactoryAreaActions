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
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void Run_Implementation() override;
	virtual void OnCancel_Implementation() override;
	virtual void EquipmentEquipped(class AAAEquipment* Equipment) override;

	void PrimaryFire();
	void ScrollUp();
	void ScrollDown();

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

	UFUNCTION(BlueprintCallable)
	void SetDeltaFromAnchorTransform(FTransform HologramTransform);

	UFUNCTION(BlueprintImplementableEvent)
    void ShowCopyWidget();
	
	UFUNCTION(BlueprintCallable)
	void Preview();
	
	UFUNCTION(BlueprintCallable)
	void Finish();

	UFUNCTION()
	void RemoveMissingItemsWidget();

	UFUNCTION(BlueprintCallable)
	void EnterPickAnchor();

	UFUNCTION(BlueprintCallable)
	void EnterPlacing();
	
protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UAACopyBuildingsComponent* CopyBuildingsComponent;

	FVector DeltaPosition;
	FRotator DeltaRotation;
	
	UPROPERTY()
	AFGBuildable* Anchor;

	UPROPERTY()
	UWidget* MissingItemsWidget;
	
public:
	UPROPERTY(BlueprintReadOnly)
	bool bIsPickingAnchor;
	
	UPROPERTY(BlueprintReadOnly)
	bool bIsPlacing;

private:
	FInputActionBinding* ScrollUpInputActionBinding;
	FInputActionBinding* ScrollDownInputActionBinding;
};
