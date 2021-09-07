#pragma once

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
	
	UFUNCTION(BlueprintPure)
	void GetDelta(FVector& OutDeltaPosition, FRotator& OutDeltaRotation) const
	{
		OutDeltaPosition = this->DeltaPosition;
		OutDeltaRotation = this->DeltaRotation;
	}

	UFUNCTION(BlueprintCallable)
	void SetDeltaFromAnchorTransform(FTransform HologramTransform);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSelectBlueprintWidget();

	UFUNCTION(BlueprintCallable)
	void NameSelected(const FString BlueprintName);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowPlaceBlueprintWidget();
	
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
