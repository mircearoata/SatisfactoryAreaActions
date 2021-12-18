﻿#pragma once

#include "CoreMinimal.h"

#include "AAAction.h"
#include "AABlueprintPlacingComponent.h"
#include "Components/Widget.h"
#include "AALoadBlueprint.generated.h"

UCLASS(Abstract, Blueprintable)
class AREAACTIONS_API AAALoadBlueprint : public AAAAction
{
	GENERATED_BODY()

public:
	AAALoadBlueprint();
	virtual void Tick(float DeltaSeconds) override;
	virtual void EnableInput(APlayerController* PlayerController) override;
	
	virtual void OnCancel_Implementation() override;

	void PrimaryFire();
	void ScrollUp();
	void ScrollDown();

	UFUNCTION(BlueprintCallable)
	void SetDelta(const FVector InDeltaPosition, const FRotator InDeltaRotation)
	{
		this->DeltaPosition = InDeltaPosition;
		this->DeltaRotation = InDeltaRotation;
	}
	
	UFUNCTION(BlueprintPure)
	void GetDelta(FVector& OutDeltaPosition, FRotator& OutDeltaRotation) const
	{
		OutDeltaPosition = this->DeltaPosition;
		OutDeltaRotation = this->DeltaRotation;
	}

	UFUNCTION(BlueprintCallable)
	void SetDeltaFromAnchorTransform(FTransform HologramTransform);

	UFUNCTION(BlueprintCallable)
	void PathSelected(const FString BlueprintPath);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowPlaceBlueprintWidget();
	
	UFUNCTION(BlueprintCallable)
	void Preview();
	
	UFUNCTION(BlueprintCallable)
	bool Finish(const TArray<UFGInventoryComponent*>& Inventories, TArray<FInventoryStack>& MissingItems);

	UFUNCTION()
	void RemoveMissingItemsWidget();

	UFUNCTION(BlueprintCallable)
	void EnterPickAnchor();

	UFUNCTION(BlueprintCallable)
	void EnterPlacing();
	
protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UAABlueprintPlacingComponent* BlueprintPlacingComponent;

	FVector DeltaPosition;
	FRotator DeltaRotation;

	int32 AnchorIdx = INDEX_NONE;
	
	UPROPERTY()
	UWidget* MissingItemsWidget;
	
	public:
	UPROPERTY(BlueprintReadOnly)
	bool bIsPickingAnchor;
	
	UPROPERTY(BlueprintReadOnly)
	bool bIsPlacing;

	UPROPERTY()
	UAABlueprint* Blueprint;
private:
	FInputActionBinding* ScrollUpInputActionBinding;
	FInputActionBinding* ScrollDownInputActionBinding;
};
