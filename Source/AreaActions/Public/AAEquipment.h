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
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActionDone);

UENUM(BlueprintType)
enum EAASelectionMode {
	SM_Corner UMETA(DisplayName="Corner"),
	SM_Bottom UMETA(DisplayName="Bottom"),
	SM_Top UMETA(DisplayName="Top"),
	SM_Building UMETA(DisplayName="Building"),
	SM_MAX UMETA(Hidden)
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

	virtual void BeginPlay() override;

	virtual void Equip(class AFGCharacterPlayer* Character) override;
	virtual void UnEquip() override;

	UFUNCTION(BlueprintCallable)
	void PrimaryFire();

	UFUNCTION(BlueprintCallable)
	void SecondaryFire();

	UFUNCTION(BlueprintCallable)
	void SetSelectionMode(const EAASelectionMode Mode) { this->SelectionMode = Mode; }
	
	UFUNCTION(BlueprintCallable)
    EAASelectionMode GetSelectionMode() const { return this->SelectionMode; }

	UFUNCTION(BlueprintCallable)
	void SelectMap();

	UFUNCTION(BlueprintCallable)
	void ClearSelection();

	UFUNCTION(BlueprintCallable)
	void RunAction(TSubclassOf<AAAAction> ActionClass);
	
	UFUNCTION(BlueprintPure)
	class AFGPlayerController* GetOwningController() const;
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void AddActionWidget(UWidget* Widget);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
    void RemoveActionWidget(UWidget* Widget);

	void ActionDone();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta = (AutoCreateRefTerm = "message"))
    UWidget* CreateActionMessageOk(const FText& Message, const FOnMessageOk& OnOkClicked);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta = (AutoCreateRefTerm = "message"))
    UWidget* CreateActionMessageYesNo(const FText& Message, const FOnMessageYes& OnYesClicked, const FOnMessageNo& OnNoClicked);
	
protected:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void AddWidget(UFGInteractWidget* Widget);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta = (AutoCreateRefTerm = "title,message"))
	void ShowMessageOk(const FText& Title, const FText& Message);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta = (AutoCreateRefTerm = "title,message"))
	void ShowMessageOkDelegate(const FText& Title, const FText& Message, const FOnMessageOk& OnOkClicked);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta = (AutoCreateRefTerm = "title,message"))
	void ShowMessageYesNoDelegate(const FText& Title, const FText& Message, const FOnMessageYes& OnYesClicked, const FOnMessageNo& OnNoClicked);

	UFUNCTION(BlueprintCallable)
	void UpdateExtraActors() const;

	UFUNCTION(BlueprintImplementableEvent)
	void DelayedUpdateExtraActors();

public:
	UPROPERTY(BlueprintAssignable)
	FOnActionDone OnActionDone;

private:
	bool RaycastMouseWithRange(FHitResult & OutHitResult, bool bIgnoreCornerIndicators = false, bool bIgnoreWallIndicators = false, bool bIgnoreHeightIndicators = false, TArray<AActor*> OtherIgnoredActors = TArray<AActor*>()) const;

	void AddCorner(FVector2D Location);
	void RemoveCorner(int CornerIdx);
	void UpdateHeight();

	AAACornerIndicator* CreateCornerIndicator(FVector2D Location) const;
	AAAWallIndicator* CreateWallIndicator(FVector2D From, FVector2D To) const;

	void GetAllActorsInArea(TArray<AActor*>& OutActors);

protected:
	UPROPERTY(BlueprintReadOnly)
	AAAAction* CurrentAction;

private:
	UPROPERTY()
	TArray<AActor*> ExtraActors;

	TArray<FVector2D> AreaCorners;
	float AreaMinZ;
	float AreaMaxZ;
	EAASelectionMode SelectionMode;
	
	UPROPERTY()
	TArray<AAACornerIndicator*> CornerIndicators;
	
	UPROPERTY()
	TArray<AAAWallIndicator*> WallIndicators;
	
	UPROPERTY()
	AAAHeightIndicator* TopIndicator;

	UPROPERTY()
	AAAHeightIndicator* BottomIndicator;

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
