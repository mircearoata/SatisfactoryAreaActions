// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FGBuildable.h"
// #include "FGBuildableHologram.h"
#include "AACopyBuildingsComponent.generated.h"

USTRUCT()
struct FPreviewBuildings
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<int32, AFGBuildable*> mBuildings;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AREAACTIONS_API UAACopyBuildingsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAACopyBuildingsComponent();

	bool SetActors(TArray<AActor*>& actors, TArray<AFGBuildable*>& out_buildingsWithIssues);
	bool SetBuildings(TArray<AFGBuildable*>& buildings, TArray<AFGBuildable*>& out_buildingsWithIssues);
	bool ValidateBuildings(TArray<AFGBuildable*>& out_buildingsWithIssues);

	int AddCopy(FVector offset, FRotator rotation);
	void MoveCopy(int copyId, FVector offset, FRotator rotation);
	void RemoveCopy(int copyId);

	void Finish();

private:
	bool ValidateObject(UObject* buildable);

	void FixReferencesToBuilding(UObject* from, UObject* to, UObject* referenceFrom, UObject* referenceTo) const;

private:
	int32 mCurrentId;
	
	UPROPERTY()
	TArray<AFGBuildable*> mOriginalBuildings;

	TArray<TPair<FVector, FRotator>> mCopyLocations;

	UPROPERTY()
	TMap<AFGBuildable*, FPreviewBuildings> mBuildingsPreview;

	TSet<UProperty*> ValidCheckSkipProperties;
};
