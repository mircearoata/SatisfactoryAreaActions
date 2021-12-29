// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AABlueprint.h"
#include "AAObjectReferenceArchive.h"
#include "Components/ActorComponent.h"
#include "Buildables/FGBuildable.h"
#include "Hologram/FGBuildableHologram.h"

#include "AABlueprintPlacingComponent.generated.h"

USTRUCT()
struct FAABlueprintObjectMap
{
	GENERATED_BODY()

	template<typename T>
	T* GetObjectReference(const FAAObjectReference ObjectReference, const bool IncludeOriginal = true) const
	{
		checkf(ObjectReference.Idx < Objects.Num(), TEXT("Tried to get ObjectReference that was not created yet"));
		if (ObjectReference.Idx == -1)
		{
			// is not copied, reference the same object
			if(IncludeOriginal)
				return FindObject<UObject>(nullptr, *ObjectReference.RelativePath, false);
			return nullptr;
		}
		if (ObjectReference.RelativePath == TEXT("None"))
		{
			// It is one of the copied objects
			return Objects[ObjectReference.Idx];
		}
		// look up the object by fully qualified pathname
		return FindObject<UObject>(Objects[ObjectReference.Idx], *ObjectReference.RelativePath, false);
	}

	template<class T>
	FORCEINLINE T* GetObject(int32 Idx) const
	{
		return Cast<T>(Objects[Idx]);
	}

	FORCEINLINE void AddObject(int32 Idx, UObject* Copy)
	{
		Objects.Add(Idx, Copy);
	}

	FORCEINLINE TMap<int32, UObject*> GetObjects() const { return Objects; }

private:
	UPROPERTY()
	TMap<int32, UObject*> Objects;
};

USTRUCT()
struct FAABlueprintPreview
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<int32, AFGBuildableHologram*> Holograms;
};

USTRUCT()
struct FBlueprintPlaceLocation
{
	GENERATED_BODY()

	FBlueprintPlaceLocation() : FBlueprintPlaceLocation(FVector::ZeroVector, FRotator::ZeroRotator, FVector::ZeroVector) {}

	FBlueprintPlaceLocation(const FVector& Offset, const FRotator& Rotation, const FVector& RotationCenter, const bool Relative = false)
		: Offset(Offset)
		, Rotation(Rotation)
		, RotationCenter(RotationCenter)
		, Relative(Relative) {}

	bool operator==(FBlueprintPlaceLocation& Other) const
	{
		return Offset == Other.Offset && Rotation == Other.Rotation && RotationCenter == Other.RotationCenter && Relative == Other.Relative;
	}
	
	FVector Offset;
	FRotator Rotation;
	FVector RotationCenter;
	bool Relative;
};

UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class AREAACTIONS_API UAABlueprintPlacingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAABlueprintPlacingComponent();

	bool SetBlueprint(UAABlueprint* Blueprint);

	UFUNCTION(BlueprintPure)
	FORCEINLINE FVector GetBuildingsCenter() const { return Blueprint->GetBoundingBox().Center; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE FAARotatedBoundingBox GetBounds() const { return Blueprint->GetBoundingBox(); }


	UFUNCTION(BlueprintCallable)
	int AddCopy(FVector Offset, FRotator Rotation, FVector RotationCenter, bool Relative = true);
	FORCEINLINE int AddCopy(const FVector Offset, const FRotator Rotation, const bool Relative = true) { return this->AddCopy(Offset, Rotation, GetBuildingsCenter(), Relative); }

	UFUNCTION(BlueprintCallable)
	void MoveCopy(int CopyId, FVector Offset, FRotator Rotation, FVector RotationCenter, bool Relative = true);
	FORCEINLINE void MoveCopy(const int CopyId, const FVector Offset, const FRotator Rotation, const bool Relative = true) { this->MoveCopy(CopyId, Offset, Rotation, GetBuildingsCenter(), Relative); }

	UFUNCTION(BlueprintCallable)
	void RemoveCopy(int CopyId);


	UFUNCTION(BlueprintCallable)
	bool Finish(const TArray<UFGInventoryComponent*> Inventories, TMap<TSubclassOf<UFGItemDescriptor>, int32>& OutMissingItems);

	UFUNCTION(BlueprintCallable)
	AFGBuildableHologram* GetPreviewHologram(int CopyId, int32 Buildable);
	
	UFUNCTION(BlueprintCallable)
	void GetAllPreviewHolograms(TArray<AFGBuildableHologram*>& OutPreviewHolograms);
	
	UFUNCTION(BlueprintCallable)
	int32 GetHologramObjectIdx(AFGBuildableHologram* Hologram);

	UFUNCTION(BlueprintCallable)
	TMap<TSubclassOf<UFGItemDescriptor>, int32> GetRequiredItems();
	
	UFUNCTION(BlueprintCallable)
	void SetHologramState(EHologramMaterialState NewState);

private:
	FTransform TransformAroundPoint(FTransform OriginalTransform, FVector Offset, FRotator Rotation, FVector RotationCenter);
	
	void FixReferencesForCopy(const FAABlueprintObjectMap& Copy);
	
private:
	UPROPERTY()
	UAABlueprint* Blueprint;

	int32 CurrentId;

	TMap<int32, FBlueprintPlaceLocation> CopyLocations;

	UPROPERTY()
	TMap<int32, FAABlueprintPreview> Preview;
};
