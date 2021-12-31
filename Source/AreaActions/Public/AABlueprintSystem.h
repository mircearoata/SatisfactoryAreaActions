// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AABlueprint.h"
#include "AABlueprintSystem.generated.h"

/**
 * 
 */
UCLASS()
class AREAACTIONS_API UAABlueprintSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UAABlueprintSystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void DiscoverBlueprints();
	UFUNCTION(BlueprintPure)
	FORCEINLINE TMap<FString, FAABlueprintHeader> GetCachedBlueprints() const { return CachedBlueprints; }
	
	bool SaveBlueprint(const FString BlueprintName, UAABlueprint* Blueprint);
	UAABlueprint* LoadBlueprint(const FString BlueprintName);
	static FString GetBlueprintsDir();
	static FString GetBlueprintPath(const FString Name);

	UFUNCTION(BlueprintPure)
	static FORCEINLINE FString GetBlueprintName(const FAABlueprintHeader& Header) { return Header.BlueprintName; }

	UFUNCTION(BlueprintPure)
	static FORCEINLINE UTexture2D* GetBlueprintIcon(const FAABlueprintHeader& Header) { return Header.Icon; }

	UFUNCTION(BlueprintPure)
	static FORCEINLINE FAARotatedBoundingBox GetBoundingBox(const FAABlueprintHeader& Header) { return Header.BoundingBox; }

	UFUNCTION(BlueprintPure)
	static FORCEINLINE TMap<TSubclassOf<UFGItemDescriptor>, int32> GetBuildCosts(const FAABlueprintHeader& Header) { return Header.BuildCosts; }

	UFUNCTION(BlueprintPure)
	static FORCEINLINE TMap<TSubclassOf<UFGItemDescriptor>, int32> GetOtherItems(const FAABlueprintHeader& Header) { return Header.OtherItems; }
private:
	bool SaveBlueprintInternal(const FString BlueprintName, UAABlueprint* Blueprint) const;
	UPROPERTY()
	TMap<FString, FAABlueprintHeader> CachedBlueprints;	
};
