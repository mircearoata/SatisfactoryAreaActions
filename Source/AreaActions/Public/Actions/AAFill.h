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
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Amount;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool Reversed;
    
    static FFillAxis None;
};

USTRUCT(BlueprintType)
struct FFillCount
{
    GENERATED_BODY()
    FFillCount(const FFillAxis InX, const FFillAxis InY, const FFillAxis InZ): X(InX), Y(InY), Z(InZ) {}
    FFillCount(): X(FFillAxis::None), Y(FFillAxis::None), Z(FFillAxis::None) {}
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FFillAxis X;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FFillAxis Y;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FFillAxis Z;
};

UENUM(BlueprintType)
enum class EFillDirection : uint8
{
    XY,
    XZ,
    YZ,
    XYZ
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
	virtual void Tick(float DeltaSeconds) override;
    virtual void EnableInput(APlayerController* PlayerController) override;
    
    virtual void OnCancel_Implementation() override;

    void PrimaryFire();
    void ScrollUp();
    void ScrollDown();
    
    UFUNCTION(BlueprintCallable)
    void SetSettings(const FFillCount InCount, const FVector InBorder, const FVector2D InRamp)
    {
        this->Count = InCount;
        this->Border = InBorder;
        this->Ramp = InRamp;
    }
    
    UFUNCTION(BlueprintPure)
    void GetSettings(FFillCount& OutCount, FVector& OutBorder, FVector2D& OutRamp) const
    {
        OutCount = this->Count;
        OutBorder = this->Border;
        OutRamp = this->Ramp;
    }
	
    UFUNCTION(BlueprintCallable)
    void Preview();
	
    UFUNCTION(BlueprintCallable)
    bool Finish(const TArray<UFGInventoryComponent*>& Inventories, TArray<FInventoryStack>& MissingItems);

    UFUNCTION()
    void RemoveMissingItemsWidget();
   
    UFUNCTION(BlueprintCallable)
    void EnterPlacing();

protected:
    UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
    UAACopyBuildingsComponent* CopyBuildingsComponent;

    UPROPERTY(BlueprintReadWrite)
    FVector AreaSize;
    
    FFillCount Count;
    FVector Border;
    FVector2D Ramp;
    
    TMap<FIntVector, int32> CopyId;

    UPROPERTY()
    UWidget* MissingItemsWidget;

public:	
    UPROPERTY(BlueprintReadOnly)
    bool bIsPlacing;

    UPROPERTY(BlueprintReadOnly)
    EFillDirection FillDirection;

private:
    FInputActionBinding* ScrollUpInputActionBinding;
    FInputActionBinding* ScrollDownInputActionBinding;
};
