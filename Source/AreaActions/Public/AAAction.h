// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FGInventoryLibrary.h"
#include "AAAction.generated.h"

UCLASS(Abstract, hidecategories = (Rendering, Replication, Input, Actor, Collision, "Actor Tick", LOD, Cooking))
class AREAACTIONS_API AAAAction : public AActor
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent)
	void Run();

	void InternalRun();
	
	UFUNCTION(BlueprintNativeEvent)
	void PostRun();

	UFUNCTION(BlueprintCallable)
	void Done();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PrimaryFire();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SecondaryFire();

	FORCEINLINE void SetActors(TArray<AActor*> actors) { this->mActors = actors; }
	FORCEINLINE void SetAAEquipment(class AAAEquipment* equipment) { this->mAAEquipment = equipment; }
public:
	UPROPERTY(BlueprintReadOnly)
	class AAAEquipment* mAAEquipment;

protected:
	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> mActors;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	FText ActionName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action", meta = (multiline = true))
	FText ActionDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	bool CloseAfterRun;
};
