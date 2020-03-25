// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AACornerIndicator.h"
#include "AAWallIndicator.h"
#include "AAHeightIndicator.h"
#include "AAAction.h"
#include "CoreMinimal.h"
#include "Equipment/FGEquipment.h"
#include "FGInteractWidget.h"
#include "AAEquipment.generated.h"

UENUM()
enum EAASelectionMode {
	SM_CORNER,
	SM_BOTTOM,
	SM_TOP,
	SM_BUILDING
};

/**
 * 
 */
UCLASS(Abstract)
class AREAACTIONS_API AAAEquipment : public AFGEquipment
{
	GENERATED_BODY()

public:
	AAAEquipment();

	void BeginPlay() override;

	void Equip(class AFGCharacterPlayer* character) override;

	UFUNCTION(BlueprintCallable)
	void PrimaryFire();

	UFUNCTION(BlueprintCallable)
	void SecondaryFire();

	UFUNCTION(BlueprintCallable)
	void SetSelectionMode(EAASelectionMode mode) { this->mSelectionMode = mode; }

	UFUNCTION(BlueprintCallable)
	void SelectMap();

	UFUNCTION(BlueprintCallable)
	void ClearSelection();

	UFUNCTION(BlueprintCallable)
	void RunAction(TSubclassOf<AAAAction> actionClass);

private:
	bool RaycastMouseWithRange(FHitResult & out_hitResult, bool ignoreCornerIndicators = false, bool ignoreWallIndicators = false, bool ignoreHeightIndicators = false, TArray<AActor*> otherIgnoredActors = TArray<AActor*>());

	void AddCorner(FVector2D location);
	void RemoveCorner(int cornerIdx);
	void UpdateHeight();

	AAACornerIndicator* CreateCornerIndicator(FVector2D location);
	AAAWallIndicator* CreateWallIndicator(FVector2D from, FVector2D to);

	void UpdateExtraActors();

	void GetAllActorsInArea(TArray<AActor*>& out_actors);
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
	float MinZ = -350000.0;
	
	UPROPERTY(EditDefaultsOnly)
	float MaxZ = 450000.0;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UFGInteractWidget> MainWidgetClass;
private:
	UPROPERTY()
	TArray<AActor*> mExtraActors;

	TArray<FVector2D> mAreaCorners;
	float mAreaMinZ;
	float mAreaMaxZ;
	EAASelectionMode mSelectionMode;

	UPROPERTY()
	TArray<AAACornerIndicator*> mCornerIndicators;
	
	UPROPERTY()
	TArray<AAAWallIndicator*> mWallIndicators;
	
	UPROPERTY()
	AAAHeightIndicator* mTopIndicator;

	UPROPERTY()
	AAAHeightIndicator* mBottomIndicator;
};
