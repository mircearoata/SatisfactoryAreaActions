// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AAAction.h"
#include "Components/Widget.h"


#include "AAClearInventories.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class AREAACTIONS_API AAAClearInventories : public AAAAction
{
	GENERATED_BODY()

public:	
	UFUNCTION(BlueprintCallable)
	int32 ClearInventories();
	
protected:
	UPROPERTY()
	UWidget* ConfirmWidget;
};
