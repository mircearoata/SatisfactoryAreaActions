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

DECLARE_DYNAMIC_DELEGATE(FOnMessageOk);
DECLARE_DYNAMIC_DELEGATE(FOnMessageYes);
DECLARE_DYNAMIC_DELEGATE(FOnMessageNo);

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
	void UnEquip() override;

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
	
	UFUNCTION(BlueprintPure)
	class AFGPlayerController* GetOwningController();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void AddWidget(UFGInteractWidget* widget);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta = (AutoCreateRefTerm = "title,message"))
	void ShowMessageOk(const FText& title, const FText& message);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta = (AutoCreateRefTerm = "title,message"))
	void ShowMessageOkDelegate(const FText& title, const FText& message, const FOnMessageOk& onOkClicked);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta = (AutoCreateRefTerm = "title,message"))
	void ShowMessageYesNoDelegate(const FText& title, const FText& message, const FOnMessageYes& onYesClicked, const FOnMessageNo& onNoClicked);

	UFUNCTION(BlueprintCallable)
	void UpdateExtraActors();

	UFUNCTION(BlueprintImplementableEvent)
	void DelayedUpdateExtraActors();

public:
	void ActionDone();

private:


private:
	bool RaycastMouseWithRange(FHitResult & out_hitResult, bool ignoreCornerIndicators = false, bool ignoreWallIndicators = false, bool ignoreHeightIndicators = false, TArray<AActor*> otherIgnoredActors = TArray<AActor*>());

	void AddCorner(FVector2D location);
	void RemoveCorner(int cornerIdx);
	void UpdateHeight();

	AAACornerIndicator* CreateCornerIndicator(FVector2D location);
	AAAWallIndicator* CreateWallIndicator(FVector2D from, FVector2D to);

	void GetAllActorsInArea(TArray<AActor*>& out_actors);

protected:
	UPROPERTY(BlueprintReadOnly)
	AAAAction* mCurrentAction;

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

public:
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
	
	UPROPERTY(EditDefaultsOnly)
	FText WidgetTitle = FText::FromString(TEXT("Area Actions"));
	
	UPROPERTY(EditDefaultsOnly)
	FText AreaNotSetMessage = FText::FromString(TEXT("Needs at least 3 corners, or at least one selected building!"));
	
	UPROPERTY(EditDefaultsOnly)
	FText ConflictingActionRunningMessage = FText::FromString(TEXT("Another action is already running!"));
};
