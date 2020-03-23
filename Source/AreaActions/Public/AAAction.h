// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AAAction.generated.h"

UCLASS(hidecategories = (Rendering, Replication, Input, Actor, Collision, "Actor Tick", LOD, Cooking))
class AREAACTIONS_API AAAAction : public AActor
{
	GENERATED_BODY()
public:
	virtual void Run(TArray<AActor*> actors);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	FText ActionName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action", meta = (multiline = true))
	FText ActionDescription;
};
