// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AACornerIndicator.h"
#include "AAWallIndicator.h"
#include "AAHeightIndicator.h"
#include "AAAction.h"
#include "CoreMinimal.h"

#include "AALocalPlayerSubsystem.h"
#include "Equipment/FGEquipment.h"
#include "UI/FGInteractWidget.h"
#include "AAEquipment.generated.h"

DECLARE_DYNAMIC_DELEGATE(FOnMessageOk);
DECLARE_DYNAMIC_DELEGATE(FOnMessageYes);
DECLARE_DYNAMIC_DELEGATE(FOnMessageNo);

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
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void OpenMainWidget();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void CloseMainWidget();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta = (AutoCreateRefTerm = "message"))
	UWidget* CreateActionMessageOk(const FText& Message, const FOnMessageOk& OnOkClicked);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta = (AutoCreateRefTerm = "message"))
	UWidget* CreateActionMessageYesNo(const FText& Message, const FOnMessageYes& OnYesClicked, const FOnMessageNo& OnNoClicked);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta = (AutoCreateRefTerm = "title,message"))
	void ShowMessageOk(const FText& Title, const FText& Message);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta = (AutoCreateRefTerm = "title,message"))
	void ShowMessageOkDelegate(const FText& Title, const FText& Message, const FOnMessageOk& OnOkClicked);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, meta = (AutoCreateRefTerm = "title,message"))
	void ShowMessageYesNoDelegate(const FText& Title, const FText& Message, const FOnMessageYes& OnYesClicked, const FOnMessageNo& OnNoClicked);
	
	UFUNCTION(BlueprintCallable)
	void AddWidget(UFGInteractWidget* Widget);
	
	UFUNCTION(BlueprintCallable)
	void AddActionWidget(UWidget* Widget);
	
	UFUNCTION(BlueprintCallable)
	void RemoveActionWidget(UWidget* Widget);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void UpdateDisplayedActionWidget();
	
	UFUNCTION(BlueprintPure)
	class AFGPlayerController* GetOwningController() const;

	bool RaycastMouseWithRange(FHitResult & OutHitResult, bool bIgnoreCornerIndicators = false, bool bIgnoreWallIndicators = false, bool bIgnoreHeightIndicators = false, TArray<AActor*> OtherIgnoredActors = TArray<AActor*>()) const;

	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta=(DisplayName = "RaycastMouseWithRange", AutoCreateRefTerm = "OtherIgnoredActors"))
	FORCEINLINE bool K2_RaycastMouseWithRange(FHitResult & OutHitResult, bool bIgnoreCornerIndicators, bool bIgnoreWallIndicators, bool bIgnoreHeightIndicators, TArray<AActor*> OtherIgnoredActors) const
	{
		return RaycastMouseWithRange(OutHitResult, bIgnoreCornerIndicators, bIgnoreWallIndicators, bIgnoreHeightIndicators, OtherIgnoredActors);
	}
	
	UFUNCTION(BlueprintCallable)
	void RunAction(TSubclassOf<AAAAction> ActionClass);

private:
	UFUNCTION()
	void OnActionDone();

public:
	UPROPERTY(BlueprintReadOnly)
	UAALocalPlayerSubsystem* LocalPlayerSubsystem;

private:	
	EAASelectionMode SelectionMode;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UFGInteractWidget> MainWidgetClass;
	
	UPROPERTY(EditDefaultsOnly)
	FText WidgetTitle = FText::FromString(TEXT("Area Actions"));
	
	UPROPERTY(EditDefaultsOnly)
	float MaxRaycastDistance = 50000;
};
