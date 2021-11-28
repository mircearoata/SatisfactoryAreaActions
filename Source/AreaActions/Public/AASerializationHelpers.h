// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AASerializationHelpers.generated.h"

/**
 * 
 */
UCLASS()
class AREAACTIONS_API UAASerializationHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION()
	static void CallPreSaveGame(UObject* Object);

	UFUNCTION()
	static void CallPostSaveGame(UObject* Object);

	UFUNCTION()
	static void CallPreLoadGame(UObject* Object);

	UFUNCTION()
	static void CallPostLoadGame(UObject* Object);
};
