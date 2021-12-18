#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"
#include "AACornerIndicator.h"
#include "AAWallIndicator.h"
#include "AAHeightIndicator.h"
#include "AAAction.h"
#include "CoreMinimal.h"
#include "Equipment/FGBuildGun.h"
#include "UI/FGInteractWidget.h"

#include "AALocalPlayerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActionDone);

UCLASS()
class AREAACTIONS_API UAALocalPlayerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
public:
	UAALocalPlayerSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable)
	bool RunAction(TSubclassOf<AAAAction> ActionClass, FText& Error);

	void ActionDone();
protected:
	UFUNCTION(BlueprintCallable)
	void UpdateExtraActors() const;

	void DelayedUpdateExtraActors();

	UFUNCTION()
	void OnBuildGunStateChanged(EBuildGunState NewState);

public:
	void AddCorner(FVector2D Location);
	void RemoveCorner(int CornerIdx);
	void UpdateHeight();

	UFUNCTION(BlueprintCallable)
	void SelectMap();

	UFUNCTION(BlueprintCallable)
	void ClearSelection();

	bool RaycastMouseWithRange(FHitResult & OutHitResult, bool bIgnoreCornerIndicators = false, bool bIgnoreWallIndicators = false, bool bIgnoreHeightIndicators = false, TArray<AActor*> OtherIgnoredActors = TArray<AActor*>()) const;

	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta=(DisplayName = "RaycastMouseWithRange", AutoCreateRefTerm = "OtherIgnoredActors"))
	FORCEINLINE bool K2_RaycastMouseWithRange(FHitResult & OutHitResult, bool bIgnoreCornerIndicators, bool bIgnoreWallIndicators, bool bIgnoreHeightIndicators, TArray<AActor*> OtherIgnoredActors) const
	{
		return RaycastMouseWithRange(OutHitResult, bIgnoreCornerIndicators, bIgnoreWallIndicators, bIgnoreHeightIndicators, OtherIgnoredActors);
	}

	AAACornerIndicator* CreateCornerIndicator(FVector2D Location) const;
	AAAWallIndicator* CreateWallIndicator(FVector2D From, FVector2D To) const;

	void GetAllActorsInArea(TArray<AActor*>& OutActors);
	
	UPROPERTY(BlueprintReadOnly)
	AAAAction* CurrentAction;

	UFUNCTION(BlueprintCallable)
	void ToggleBuildMenu();

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

	friend class AAASelectionActor;
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
	
	UPROPERTY(EditDefaultsOnly)
	float MaxRaycastDistance = 50000;
};
