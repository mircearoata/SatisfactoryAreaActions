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
	TArray<AFGBuildable*> mBuildings;
};

USTRUCT()
struct FObjectArrayContainer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<UObject*> Objects;
};

USTRUCT()
struct FWeakObjectContainer
{
	GENERATED_BODY()

	FWeakObjectPtr Object;
};

USTRUCT()
struct FWeakObjectArrayContainer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FWeakObjectContainer> Objects;
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
	FORCEINLINE void SetAAEquipment(class AAAEquipment* equipment) { this->mAAEquipment = equipment; }

	int AddCopy(FVector offset, FRotator rotation);
	void RemoveCopy(int copyId);

	void Finish();

private:
	bool ValidateObject(UObject* buildable);

	void FixReferencesToBuilding(UObject* from, UObject* to, UObject* referenceFrom, UObject* referenceTo) const;

private:
	UPROPERTY()
	class AAAEquipment* mAAEquipment;

	int mCurrentId = 0;

	UPROPERTY()
	TArray<AFGBuildable*> mOriginalBuildings;

	TArray<TPair<FVector, FRotator>> mCopyLocations;
	TArray<int> mIdIdx;

	UPROPERTY()
	TMap<AFGBuildable*, FPreviewBuildings> mBuildingsPreview;

	TSet<UProperty*> ValidCheckSkipProperties;
};
