// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Styling/SlateBrush.h"
#include "Templates/SubclassOf.h"
#include "AAActionCategory.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class AREAACTIONS_API UAAActionCategory : public UObject
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintPure, Category = "Action Category")
    static FText GetCategoryName(TSubclassOf<UAAActionCategory> InClass);

    UFUNCTION(BlueprintPure, Category = "Action Category")
    static FSlateBrush GetCategoryIcon(TSubclassOf<UAAActionCategory> InClass);

private:
    UPROPERTY(EditDefaultsOnly, Category="Action Category")
    FText Name;

    UPROPERTY(EditDefaultsOnly, Category="Action Category")
    FSlateBrush Icon;
};
