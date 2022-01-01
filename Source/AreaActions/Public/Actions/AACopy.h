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
	virtual void EnableInput(APlayerController* PlayerController) override;
	
	virtual void OnCancel_Implementation() override;

	void PrimaryFire();
	void OpenMenu();
	void ScrollUp();
	void ScrollDown();

	UFUNCTION(BlueprintCallable)
	void CreateNewCopy();

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
	
	UFUNCTION(BlueprintCallable)
	void Preview();
	
	UFUNCTION(BlueprintCallable)
	bool Finish();
	
	UFUNCTION(BlueprintPure)
	bool CanFinish() const;
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintPure)
	TArray<UFGInventoryComponent*> GetInventories() const;

	UFUNCTION()
	void RemoveMissingItemsWidget();

	UFUNCTION(BlueprintCallable)
	void EnterPickAnchor();

	UFUNCTION(BlueprintCallable)
	void EnterPlacing();
	
protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UAACopyBuildingsComponent* CopyBuildingsComponent;

	int32 CurrentCopy;

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
