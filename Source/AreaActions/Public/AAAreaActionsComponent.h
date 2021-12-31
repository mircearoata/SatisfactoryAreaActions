#pragma once

#include "AACornerIndicator.h"
#include "AAWallIndicator.h"
#include "AAHeightIndicator.h"
#include "AAAction.h"
#include "CoreMinimal.h"
#include "FGPlayerController.h"
#include "Equipment/FGBuildGun.h"

#include "AAAreaActionsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActionDone);

UCLASS(Within=FGBuildGun)
class AREAACTIONS_API UAAAreaActionsComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UAAAreaActionsComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

	UPROPERTY(BlueprintReadWrite)
	class AAASelectionActor* SelectionActor;

	UFUNCTION(BlueprintCallable)
	void ToggleBuildMenu();

	UFUNCTION(BlueprintCallable)
	void HideBuildMenu();

	UFUNCTION(BlueprintCallable)
	void ShowBuildMenu();

	UFUNCTION(BlueprintCallable)
	FORCEINLINE AFGPlayerController* GetPlayerController() const { return static_cast<AFGPlayerController*>(GetPlayerCharacter()->GetController()); }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE AFGCharacterPlayer* GetPlayerCharacter() const { return static_cast<AFGCharacterPlayer*>(GetOuterAFGBuildGun()->GetOwner()); }
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE TArray<AAACornerIndicator*> GetCornerIndicators() { return CornerIndicators; }
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE TArray<AAAWallIndicator*> GetWallIndicators() { return WallIndicators; }
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE AAAHeightIndicator* GetTopIndicator() { return TopIndicator; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE AAAHeightIndicator* GetBottomIndicator() { return BottomIndicator; }
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
