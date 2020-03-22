// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AACornerIndicator.h"
#include "AAWallIndicator.h"
#include "AAHeightIndicator.h"
#include "CoreMinimal.h"
#include "Equipment/FGEquipment.h"
#include "AAEquipment.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class AREAACTIONS_API AAAEquipment : public AFGEquipment
{
	GENERATED_BODY()

public:
	AAAEquipment();

	UFUNCTION(BlueprintCallable)
	void PrimaryFire();

	UFUNCTION(BlueprintCallable)
	void SecondaryFire();

private:
	bool RaycastMouseWithRange(FHitResult & out_hitResult, bool ignoreCornerIndicators = false, bool ignoreWallIndicators = false, bool ignoreHeightIndicators = false, TArray<AActor*> otherIgnoredActors = TArray<AActor*>());

	void AddCorner(FVector location);
	void RemoveCorner(int cornerIdx);

	AAACornerIndicator* CreateCornerIndicator(FVector location);
	AAAWallIndicator* CreateWallIndicator(FVector from, FVector to);
private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AAACornerIndicator> CornerIndicatorClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AAAWallIndicator> WallIndicatorClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AAAHeightIndicator> HeightIndicatorClass;
	
	UPROPERTY(EditDefaultsOnly)
	float MaxRaycastDistance = 50000;
	
	UPROPERTY(EditDefaultsOnly)
	float MinZ = 450000.0;
	
	UPROPERTY(EditDefaultsOnly)
	float MaxZ = -350000.0;

private:
	TArray<FVector> mAreaCorners;
	float mAreaMinZ;
	float mAreaMaxZ;

	UPROPERTY()
	TArray<AAACornerIndicator*> mCornerIndicators;
	
	UPROPERTY()
	TArray<AAAWallIndicator*> mWallIndicators;
	
	UPROPERTY()
	AAAHeightIndicator* mTopIndicator;

	UPROPERTY()
	AAAHeightIndicator* mBottomIndicator;
};
