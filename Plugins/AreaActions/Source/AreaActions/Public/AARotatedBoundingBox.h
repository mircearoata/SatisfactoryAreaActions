#pragma once

#include "AARotatedBoundingBox.generated.h"

USTRUCT(Blueprintable)
struct FAARotatedBoundingBox
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FVector Center;
	UPROPERTY(BlueprintReadWrite)
	FVector Extents;
	/** Only has yaw */
	UPROPERTY(BlueprintReadWrite)
	FRotator Rotation;

	FVector GetCorner(uint32 CornerNum) const;
	
	/** Store / load data */
	friend FArchive& operator<< (FArchive& Ar, FAARotatedBoundingBox& RotatedBoundingBox);
};
