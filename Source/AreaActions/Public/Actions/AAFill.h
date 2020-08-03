// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AAAction.h"
#include "AACopyBuildingsComponent.h"
#include "AAFill.generated.h"

USTRUCT(BlueprintType)
struct FFillAxis
{
    GENERATED_BODY()
    FFillAxis(const int32 InAmount, const bool InReversed): Amount(InAmount), Reversed(InReversed) {}
    FFillAxis() { Amount = 1; Reversed = false; }

    int32 Amount;
    bool Reversed;
    
    static FFillAxis None;
};

USTRUCT(BlueprintType)
struct FFillCount
{
    GENERATED_BODY()
    FFillCount(const FFillAxis InX, const FFillAxis InY, const FFillAxis InZ): X(InX), Y(InY), Z(InZ) {}
    FFillCount(): X(FFillAxis::None), Y(FFillAxis::None), Z(FFillAxis::None) {}
    
    FFillAxis X;
    FFillAxis Y;
    FFillAxis Z;
};

/**
* 
*/
UCLASS(Abstract, Blueprintable)
class AREAACTIONS_API AAAFill : public AAAAction
{
    GENERATED_BODY()

public:
    AAAFill();
    virtual void Run_Implementation() override;
    
    UFUNCTION(BlueprintCallable)
    void SetSettings(const FFillCount NewCount, const FVector NewBorder, const FVector NewRamp)
    {
        this->Count = NewCount;
        this->Border = NewBorder;
        this->Ramp = NewRamp;
    }
	
    UFUNCTION(BlueprintCallable)
    void Preview();
	
    UFUNCTION(BlueprintCallable)
    void Finish();
    
private:
    UPROPERTY()
    UAACopyBuildingsComponent* CopyBuildingsComponent;

    FVector AreaSize;
    
    FFillCount Count;
    FVector Border;
    FVector Ramp;
    
    TMap<FIntVector, int32> CopyId;
};
