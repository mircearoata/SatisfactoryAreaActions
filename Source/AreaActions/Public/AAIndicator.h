// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "AAIndicator.generated.h"

UCLASS(Abstract)
class AREAACTIONS_API AAAIndicator : public AActor
{
	GENERATED_BODY()
	
public:
	virtual void UpdateHeight(float minHeight, float maxHeight);
};
