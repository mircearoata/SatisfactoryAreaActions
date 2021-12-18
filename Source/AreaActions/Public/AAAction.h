// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AAActionCategory.h"
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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

	FORCEINLINE void SetActors(const TArray<AActor*> InActors) { this->Actors = InActors; }
	FORCEINLINE void SetSubsystem(class UAALocalPlayerSubsystem* InSubsystem) { this->LocalPlayerSubsystem = InSubsystem; }

	UFUNCTION(BlueprintPure, Category = "Action")
	static FText GetActionName(TSubclassOf<AAAAction> InClass);

	UFUNCTION(BlueprintPure, Category = "Action")
	static FText GetActionDescription(TSubclassOf<AAAAction> InClass);

	UFUNCTION(BlueprintPure, Category = "Action")
	static TSubclassOf<UAAActionCategory> GetActionCategory(TSubclassOf<AAAAction> InClass);

	UFUNCTION(BlueprintPure, Category = "Action")
	static UTexture2D* GetActionIcon(TSubclassOf<AAAAction> InClass);
	
protected:
	UPROPERTY(BlueprintReadOnly)
	class UAALocalPlayerSubsystem* LocalPlayerSubsystem;

	UPROPERTY(BlueprintReadOnly)
	TArray<AActor*> Actors;

	UPROPERTY(BlueprintReadWrite)
	UUserWidget* ActionWidget;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	FText Name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action", meta = (multiline = true))
	FText Description;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	UTexture2D* Icon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
	TSubclassOf<UAAActionCategory> Category;
};
