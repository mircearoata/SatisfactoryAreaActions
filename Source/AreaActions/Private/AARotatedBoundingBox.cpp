#include "AARotatedBoundingBox.h"

FVector FAARotatedBoundingBox::GetCorner(const uint32 CornerNum) const
{
	switch(CornerNum)
	{
	case 0:
		return Center + Rotation.RotateVector(FVector(Extents.X, Extents.Y, 0));
	case 1:
		return Center + Rotation.RotateVector(FVector(Extents.X, -Extents.Y, 0));
	case 2:
		return Center + Rotation.RotateVector(FVector(-Extents.X, -Extents.Y, 0));
	case 3:
		return Center + Rotation.RotateVector(FVector(-Extents.X, Extents.Y, 0));
	default:
		return Center;
	}
}

FArchive& operator<< (FArchive& Ar, FAARotatedBoundingBox& RotatedBoundingBox)
{
	Ar << RotatedBoundingBox.Center;
	Ar << RotatedBoundingBox.Extents;
	Ar << RotatedBoundingBox.Rotation;
	return Ar;
}