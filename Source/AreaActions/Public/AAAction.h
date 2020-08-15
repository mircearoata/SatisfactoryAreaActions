// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AAAction.generated.h"

UCLASS(Abstract, hidecategories = (Rendering, Replication, Input, Actor, Collision, "Actor Tick", LOD, Cooking))
class AREAACTIONS_API AAAAction : public AActor
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent)
	void Run();

	UFUNCTION(BlueprintCallable)
	void Done();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PrimaryFire();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SecondaryFire();

	FORCEINLINE void SetActors(const TArray<AActor*> InActors) { this->Actors = InActors; }
	FORCEINLINE void SetAAEquipment(class AAAEquipment* InEquipment) { this->AAEquipment = InEquipment; }
public:
	UPROPERTY(BlueprintReadOnly)
	class AAAEquipment* AAEquipment;

protected:
	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> Actors;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	FText ActionName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action", meta = (multiline = true))
	FText ActionDescription;
};
