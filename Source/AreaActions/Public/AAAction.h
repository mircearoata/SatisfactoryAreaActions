// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AAActionCategory.h"
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

	UFUNCTION(BlueprintNativeEvent)
    void OnCancel();
    
	UFUNCTION(BlueprintCallable)
    void Cancel();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PrimaryFire();

	FORCEINLINE void SetActors(const TArray<AActor*> InActors) { this->Actors = InActors; }
	FORCEINLINE void SetAAEquipment(class AAAEquipment* InEquipment) { this->AAEquipment = InEquipment; }

	UFUNCTION(BlueprintPure, Category = "Action")
	static FText GetActionName(TSubclassOf<AAAAction> InClass);

	UFUNCTION(BlueprintPure, Category = "Action")
	static FText GetActionDescription(TSubclassOf<AAAAction> InClass);

	UFUNCTION(BlueprintPure, Category = "Action")
	static TSubclassOf<UAAActionCategory> GetActionCategory(TSubclassOf<AAAAction> InClass);

	UFUNCTION(BlueprintPure, Category = "Action")
    static FSlateBrush GetActionIcon(TSubclassOf<AAAAction> InClass);
	
protected:
	UPROPERTY(BlueprintReadOnly)
	class AAAEquipment* AAEquipment;

	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> Actors;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	FText Name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action", meta = (multiline = true))
	FText Description;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	FSlateBrush Icon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	TSubclassOf<UAAActionCategory> Category;
};
