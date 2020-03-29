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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AREAACTIONS_API UAACopyBuildingsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void SetActors(TArray<AActor*>& actors);
	void SetBuildings(TArray<AFGBuildable*>& buildings);
	FORCEINLINE void SetAAEquipment(class AAAEquipment* equipment) { this->mAAEquipment = equipment; }

	int AddCopy(FVector offset, FRotator rotation);
	void RemoveCopy(int copyId);

	void Finish();

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
};
