// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AAActionCategory.h"
#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "GameFramework/Actor.h"
#include "AAAction.generated.h"

UCLASS(Abstract)
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

	virtual void EquipmentEquipped(class AAAEquipment* Equipment);

	virtual void EquipmentUnEquipped();

	FORCEINLINE void SetActors(const TArray<AActor*> InActors) { this->Actors = InActors; }
	FORCEINLINE void SetAAEquipment(class AAAEquipment* InEquipment) { this->AAEquipment = InEquipment; }
	FORCEINLINE void SetSubsystem(class UAALocalPlayerSubsystem* InSubsystem) { this->LocalPlayerSubsystem = InSubsystem; }

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
	class UAALocalPlayerSubsystem* LocalPlayerSubsystem;

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
