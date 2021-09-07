// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AAAction.h"
#include "AAAction.h"
#include "AACopyBuildingsComponent.h"
#include "AABlueprint.generated.h"

/**
 * 
*/
class AREAACTIONS_API FAABlueprintHeader
{
public:
	enum class HeaderFormatVersion : uint8
	{		
		// First version
		InitialVersion = 0,
		
		// -----<new versions can be added above this line>-----
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1 // Last version to use
	};

	int32 GameVersion;
	
	FAARotatedBoundingBox BoundingBox;
    TMap<TSubclassOf<UFGItemDescriptor>, int32> BuildCosts;
    TMap<TSubclassOf<UFGItemDescriptor>, int32> OtherItems;
	
	
	/** Store / load data */
	friend FArchive& operator<< (FArchive& Ar, FAABlueprintHeader& Header);
};

USTRUCT()
struct FAAObjectReference
{
	GENERATED_BODY()

public:
	int32 Idx;
	FString RelativePath;

	friend FArchive& operator<< (FArchive& Ar, FAAObjectReference& ObjectReference);
};

USTRUCT()
struct FAABlueprintObjectTOC
{
	GENERATED_BODY()

public:
	TSubclassOf<class UObject> Class;
	FName Name;
	EObjectFlags Flags;
	bool HasTransform;
	FTransform Transform;
	TSubclassOf<class UFGRecipe> BuiltWithRecipe;
	FAAObjectReference Outer;
	FAAObjectReference Owner;

	friend FArchive& operator<< (FArchive& Ar, FAABlueprintObjectTOC& BuildingTOC);
};

USTRUCT()
struct FAABlueprintObjectsData
{
	GENERATED_BODY()

public:	
	TArray<uint8> SerializedData;

	friend FArchive& operator<< (FArchive& Ar, FAABlueprintObjectsData& BuildingData);
};

UCLASS()
class AREAACTIONS_API UAABlueprint : public UObject
{
	GENERATED_BODY()
public:
	enum class BlueprintFormatVersion : uint8
	{		
		// First version
		InitialVersion = 0,
		
		// -----<new versions can be added above this line>-----
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1 // Last version to use
	};

	bool LoadBlueprint(const FString& FilePath);

	bool SetRootSet(TArray<AActor*>& Actors, TArray<AActor*> OutActorsWithIssues);

	bool SaveBlueprint(const FString& FilePath);

	FORCEINLINE TArray<FAABlueprintObjectTOC> GetObjectTOC() const { return ObjectTOC; }
	FORCEINLINE FAABlueprintObjectsData GetObjectsData() const { return ObjectsData; }
	FORCEINLINE FAARotatedBoundingBox GetBoundingBox() const { return BlueprintHeader.BoundingBox; }
	FORCEINLINE TMap<TSubclassOf<UFGItemDescriptor>, int32> GetBuildCosts() const { return BlueprintHeader.BuildCosts; }
	FORCEINLINE TMap<TSubclassOf<UFGItemDescriptor>, int32> GetOtherItems() const { return BlueprintHeader.OtherItems; }

	static FString GetBlueprintPath(FString Name);
private:
	FAARotatedBoundingBox CalculateBoundingBox();
	TMap<TSubclassOf<UFGItemDescriptor>, int32> CalculateBuildCosts();
	TMap<TSubclassOf<UFGItemDescriptor>, int32> CalculateOtherItems();
	
	void SerializeTOC();
	void SerializeData();
	void SerializeObjects();
	void SerializeBlueprint(FArchive& Ar);

protected:
	FAABlueprintHeader BlueprintHeader;

	TArray<FAABlueprintObjectTOC> ObjectTOC;
	FAABlueprintObjectsData ObjectsData;

	TArray<UObject*> ObjectsToSerialize;
};

