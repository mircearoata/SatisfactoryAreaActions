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
	virtual void EnableInput(APlayerController* PlayerController) override;
	
	virtual void OnCancel_Implementation() override;

	void PrimaryFire();
	void OpenMenu();
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
	FORCEINLINE FString GetBlueprintFileName() const { return this->BlueprintFileName; }

	UFUNCTION(BlueprintCallable)
	void SetDeltaFromAnchorTransform(FTransform HologramTransform);

	UFUNCTION(BlueprintCallable)
	bool SetPath(const FString InBlueprintName);
	
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
	UAABlueprintPlacingComponent* BlueprintPlacingComponent;

	FVector DeltaPosition;
	FRotator DeltaRotation;

	int32 AnchorIdx = INDEX_NONE;
	
	UPROPERTY()
	UWidget* MissingItemsWidget;
	
	FString BlueprintFileName;

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
