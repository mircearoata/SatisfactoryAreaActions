// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AAAction.h"
#include "FGCrate.h"
#include "FGInventoryComponent.h"
#include "Components/Widget.h"

#include "AAMassDismantle.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class AREAACTIONS_API AAAMassDismantle : public AAAAction
{
	GENERATED_BODY()

public:	
	UFUNCTION(BlueprintCallable)
	void Dismantle();

	UFUNCTION(BlueprintCallable)
	void GiveRefunds();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowGiveRefundsWidget();

protected:
	UPROPERTY(BlueprintReadOnly)
	TArray<FInventoryStack> Refunds;

	UPROPERTY()
	UWidget* RefundsWidget; 

	UPROPERTY()
	UWidget* ConfirmWidget; 

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFGCrate> CrateClass;

	UPROPERTY(EditDefaultsOnly)
	float CrateDistance = 200.0;
};
