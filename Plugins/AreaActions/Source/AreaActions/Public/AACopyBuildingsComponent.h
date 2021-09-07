// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AARotatedBoundingBox.h"
#include "Components/ActorComponent.h"
#include "Buildables/FGBuildable.h"
#include "Hologram/FGBuildableHologram.h"

#include "AACopyBuildingsComponent.generated.h"

USTRUCT()
struct FCopyMap
{
	GENERATED_BODY()

	template<typename T>
	T* GetObjectChecked(T* Obj, const bool IncludeOriginal = true) const
	{
		T* Outer = Obj;
		while(Outer && !Objects.Contains(Outer))
			Outer = Outer->GetOuter();
		if(Outer == Obj)
			return Cast<T>(Objects[Obj]);
		if(Outer)
			return FindObject<T>(Objects[Outer], *Obj->GetPathName(Outer));
		if(IncludeOriginal)
			return Obj;
		return nullptr;
	}

	template<class T>
	FORCEINLINE T* GetObject(T* Obj) const
	{
		return Cast<T>(Objects[Obj]);
	}

	FORCEINLINE void AddObject(UObject* Original, UObject* Copy)
	{
		Objects.Add(Original, Copy);
	}

private:
	UPROPERTY()
	TMap<UObject*, UObject*> Objects;
};

USTRUCT()
struct FCopyPreview
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<AFGBuildable*, AFGBuildableHologram*> Holograms;
};

USTRUCT()
struct FCopyLocation
{
	GENERATED_BODY()

	FCopyLocation() : FCopyLocation(FVector::ZeroVector, FRotator::ZeroRotator, FVector::ZeroVector) {}

	FCopyLocation(const FVector& Offset, const FRotator& Rotation, const FVector& RotationCenter, const bool Relative = false)
		: Offset(Offset)
		, Rotation(Rotation)
		, RotationCenter(RotationCenter)
		, Relative(Relative) {}

	bool operator==(FCopyLocation& Other) const
	{
		return Offset == Other.Offset && Rotation == Other.Rotation && RotationCenter == Other.RotationCenter && Relative == Other.Relative;
	}
	
	FVector Offset;
	FRotator Rotation;
	FVector RotationCenter;
	bool Relative;
};

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class AREAACTIONS_API UAACopyBuildingsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAACopyBuildingsComponent();

	bool SetActors(TArray<AActor*>& Actors, TArray<AFGBuildable*>& OutBuildingsWithIssues, FText& Error);
	bool SetBuildings(TArray<AFGBuildable*>& Buildings, TArray<AFGBuildable*>& OutBuildingsWithIssues, FText& Error);
	bool ValidateObjects(TArray<AFGBuildable*>& OutBuildingsWithIssues);

	UFUNCTION(BlueprintPure)
	FORCEINLINE FVector GetBuildingsCenter() const { return BuildingsBounds.Center; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE FAARotatedBoundingBox GetBounds() const { return BuildingsBounds; }

	int AddCopy(FVector Offset, FRotator Rotation, FVector RotationCenter, bool Relative = true);
	FORCEINLINE int AddCopy(const FVector Offset, const FRotator Rotation, const bool Relative = true) { return this->AddCopy(Offset, Rotation, GetBuildingsCenter(), Relative); }
	void MoveCopy(int CopyId, FVector Offset, FRotator Rotation, FVector RotationCenter, bool Relative = true);
	FORCEINLINE void MoveCopy(const int CopyId, const FVector Offset, const FRotator Rotation, const bool Relative = true) { this->MoveCopy(CopyId, Offset, Rotation, GetBuildingsCenter(), Relative); }
	void RemoveCopy(int CopyId);

	bool Finish(TArray<UFGInventoryComponent*> Inventories, TArray<FInventoryStack>& OutMissingItems);

	UFUNCTION(BlueprintCallable)
	AFGBuildableHologram* GetPreviewHologram(int CopyId, AFGBuildable* Buildable);
	
	UFUNCTION(BlueprintCallable)
	void GetAllPreviewHolograms(TArray<AFGBuildableHologram*>& OutPreviewHolograms);

private:
	FTransform TransformAroundPoint(FTransform OriginalTransform, FVector Offset, FRotator Rotation, FVector RotationCenter);
	
	void FixReferencesForCopy(const FCopyMap& Copy);

	void CalculateBounds();
	void SerializeOriginal();
	
	bool TryTakeItems(TArray<UFGInventoryComponent*> Inventories, TArray<FInventoryStack>& OutMissingItems);
	bool CheckItems(TMap<TSubclassOf<UFGItemDescriptor>, int32> RemainingItems, TArray<UFGInventoryComponent*> Inventories, TArray<FInventoryStack>& OutMissingItems, const bool TakeItems = false) const;
	
private:
	int32 CurrentId;
	
	UPROPERTY()
	TArray<UObject*> Original;

	TMap<int32, FCopyLocation> CopyLocations;

	UPROPERTY()
	TMap<int32, FCopyPreview> Preview;

	TSet<FProperty*> ValidCheckSkipProperties;
	
	TArray<uint8> Serialized;

	FAARotatedBoundingBox BuildingsBounds;
};
