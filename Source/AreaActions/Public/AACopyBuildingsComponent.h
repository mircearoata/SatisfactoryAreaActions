// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FGBuildable.h"
#include "SaveCustomVersion.h"
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
struct FCopyPreview
{
	GENERATED_BODY()

	UObject* GetObjectChecked(UObject* Obj, const bool IncludeOriginal = true)
	{
		UObject* Outer = Obj;
		while(Outer && !Objects.Contains(Outer))
			Outer = Outer->GetOuter();
		if(Outer == Obj)
			return Objects[Obj];
		if(Outer)
			return FindObject<UObject>(Objects[Outer], *Obj->GetPathName(Outer));
		if(IncludeOriginal)
			return Obj;
		return nullptr;
	}

	FORCEINLINE UObject* GetObject(UObject* Obj)
	{
		return Objects[Obj];
	}

	FORCEINLINE void AddObject(UObject* Original, UObject* Copy)
	{
		Objects.Add(Original, Copy);
	}

private:
	UPROPERTY()
	TMap<UObject*, UObject*> Objects;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AREAACTIONS_API UAACopyBuildingsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAACopyBuildingsComponent();

	bool SetActors(TArray<AActor*>& Actors, TArray<AFGBuildable*>& OutBuildingsWithIssues);
	bool SetBuildings(TArray<AFGBuildable*>& Buildings, TArray<AFGBuildable*>& OutBuildingsWithIssues);
	bool ValidateObjects(TArray<AFGBuildable*>& OutBuildingsWithIssues);

	FORCEINLINE FVector GetBuildingsCenter() const { return BuildingsBounds.Center; }
	FORCEINLINE FRotatedBoundingBox GetBounds() const { return BuildingsBounds; }

	int AddCopy(FVector Offset, FRotator Rotation, FVector RotationCenter, bool Relative = true);
	FORCEINLINE int AddCopy(const FVector Offset, const FRotator Rotation, const bool Relative = true) { return this->AddCopy(Offset, Rotation, GetBuildingsCenter(), Relative); }
	void MoveCopy(int CopyId, FVector Offset, FRotator Rotation, FVector RotationCenter, bool Relative = true);
	FORCEINLINE void MoveCopy(const int CopyId, const FVector Offset, const FRotator Rotation, const bool Relative = true) { this->MoveCopy(CopyId, Offset, Rotation, GetBuildingsCenter(), Relative); }
	void RemoveCopy(int CopyId);
	
	void Finish();

private:
	void FixReferencesForCopy(int CopyId);
	bool ValidateObject(UObject* Object);

	void CalculateBounds();
	void SerializeOriginal();
private:
	int32 CurrentId;
	
	UPROPERTY()
	TArray<UObject*> Original;

	TArray<TPair<FVector, FRotator>> CopyLocations;

	UPROPERTY()
	TMap<int32, FCopyPreview> Preview;

	TSet<UProperty*> ValidCheckSkipProperties;
	
	TArray<uint8> Serialized;

	FRotatedBoundingBox BuildingsBounds;
};
