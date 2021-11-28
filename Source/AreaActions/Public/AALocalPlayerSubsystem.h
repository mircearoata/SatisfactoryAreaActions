#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"
#include "AACornerIndicator.h"
#include "AAWallIndicator.h"
#include "AAHeightIndicator.h"
#include "AAAction.h"
#include "CoreMinimal.h"
#include "UI/FGInteractWidget.h"

#include "AALocalPlayerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActionDone);

UCLASS()
class AREAACTIONS_API UAALocalPlayerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
public:
	UAALocalPlayerSubsystem();

	UFUNCTION(BlueprintCallable)
	bool RunAction(TSubclassOf<AAAAction> ActionClass, AAAEquipment* Equipment, FText& Error);

	void ActionDone();
protected:
	UFUNCTION(BlueprintCallable)
	void UpdateExtraActors() const;

	void DelayedUpdateExtraActors();

public:
	void AddCorner(FVector2D Location);
	void RemoveCorner(int CornerIdx);
	void UpdateHeight();

	AAACornerIndicator* CreateCornerIndicator(FVector2D Location) const;
	AAAWallIndicator* CreateWallIndicator(FVector2D From, FVector2D To) const;

	void GetAllActorsInArea(TArray<AActor*>& OutActors);

	UPROPERTY(BlueprintAssignable)
	FOnActionDone OnActionDone;
	
	UPROPERTY(BlueprintReadOnly)
	AAAAction* CurrentAction;
	
	UPROPERTY(BlueprintReadOnly)
	TArray<UWidget*> ActionWidgets;

private:
	UPROPERTY()
	TArray<AActor*> ExtraActors;

	TArray<FVector2D> AreaCorners;
	float AreaMinZ;
	float AreaMaxZ;
	
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
	float MinZ = -350000.0;
	
	UPROPERTY(EditDefaultsOnly)
	float MaxZ = 450000.0;

private:
	UPROPERTY(EditDefaultsOnly)
	FText AreaNotSetMessage = FText::FromString(TEXT("Needs at least 3 corners, or at least one selected building!"));
	
	UPROPERTY(EditDefaultsOnly)
	FText ConflictingActionRunningMessage = FText::FromString(TEXT("Another action is already running!"));

	friend class AAAEquipment;
};
