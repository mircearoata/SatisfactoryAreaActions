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
	virtual void Init();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Init"))
	void BP_Init();

	UFUNCTION(BlueprintCallable)
	virtual void Run();

	FORCEINLINE void SetActors(TArray<AActor*> actors) { this->mActors = actors; }
	FORCEINLINE void SetAAEquipment(class AAAEquipment* equipment) { this->mAAEquipment = equipment; }
protected:
	UPROPERTY()
	TArray<AActor*> mActors;

	UPROPERTY()
	class AAAEquipment* mAAEquipment;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	FText ActionName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action", meta = (multiline = true))
	FText ActionDescription;
};
