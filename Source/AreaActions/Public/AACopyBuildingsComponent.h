// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FGBuildable.h"
// #include "FGBuildableHologram.h"
#include "AACopyBuildingsComponent.generated.h"

USTRUCT()
struct FRotatedBoundingBox
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Center;
	UPROPERTY()
	FVector Extents;
	/** Only has yaw */
	UPROPERTY()
	FRotator Rotation;

	FVector GetCorner(uint32 CornerNum) const;
};

USTRUCT()
struct FPreviewBuildings
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<int32, AFGBuildable*> Buildings;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AREAACTIONS_API UAACopyBuildingsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAACopyBuildingsComponent();

	bool SetActors(TArray<AActor*>& Actors, TArray<AFGBuildable*>& OutBuildingsWithIssues);
	bool SetBuildings(TArray<AFGBuildable*>& Buildings, TArray<AFGBuildable*>& OutBuildingsWithIssues);
	bool ValidateBuildings(TArray<AFGBuildable*>& OutBuildingsWithIssues);

	FORCEINLINE FVector GetBuildingsCenter() const { return BuildingsBounds.Center; }
	FORCEINLINE FRotatedBoundingBox GetBounds() const { return BuildingsBounds; }

	int AddCopy(FVector Offset, FRotator Rotation, FVector RotationCenter, bool Relative = true);
	FORCEINLINE int AddCopy(const FVector Offset, const FRotator Rotation, const bool Relative = true) { return this->AddCopy(Offset, Rotation, GetBuildingsCenter(), Relative); }
	void MoveCopy(int CopyId, FVector Offset, FRotator Rotation, FVector RotationCenter, bool Relative = true);
	FORCEINLINE void MoveCopy(const int CopyId, const FVector Offset, const FRotator Rotation, const bool Relative = true) { this->MoveCopy(CopyId, Offset, Rotation, GetBuildingsCenter(), Relative); }
	void RemoveCopy(int CopyId);
	
	void Finish();

private:
	bool ValidateObject(UObject* Buildable);

	void CalculateBounds();
private:
	int32 CurrentId;
	
	UPROPERTY()
	TArray<AFGBuildable*> OriginalBuildings;

	TArray<TPair<FVector, FRotator>> CopyLocations;

	UPROPERTY()
	TMap<AFGBuildable*, FPreviewBuildings> BuildingsPreview;

	TSet<UProperty*> ValidCheckSkipProperties;

	FRotatedBoundingBox BuildingsBounds;
};
