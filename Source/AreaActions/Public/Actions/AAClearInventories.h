// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AAAction.h"
#include "Widget.h"

#include "AAClearInventories.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class AREAACTIONS_API AAAClearInventories : public AAAAction
{
	GENERATED_BODY()

public:
	virtual void Run_Implementation() override;
	
	UFUNCTION()
	void ClearInventories();
	
protected:
	UPROPERTY()
	UWidget* ConfirmWidget;
};
