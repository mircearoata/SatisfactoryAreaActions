// Fill out your copyright notice in the Description page of Project Settings.


#include "AABuildingsDataHelper.h"

#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildableFactory.h"
#include "Components/SplineMeshComponent.h"

FAARotatedBoundingBox FAABuildingsDataHelper::CalculateBoundingBox(const TArray<UObject*>& Objects)
{
    TMap<float, uint32> RotationCount;
    for(UObject* Object : Objects)
        if(AActor* Actor = Cast<AActor>(Object))
        {
            TArray<USplineMeshComponent*> SplineMeshComponents;
            Actor->GetComponents<USplineMeshComponent>(SplineMeshComponents);
            if(SplineMeshComponents.Num() > 0)
                continue;
            RotationCount.FindOrAdd(FGenericPlatformMath::Fmod(FGenericPlatformMath::Fmod(Actor->GetActorRotation().Yaw, 90) + 90, 90))++;
        }

    RotationCount.ValueSort([](const uint32& A, const uint32& B) {
        return A > B;
    });

    const FRotator Rotation = FRotator(0, (*RotationCount.CreateIterator()).Key, 0);

    FVector Min = FVector(TNumericLimits<float>::Max());
    FVector Max = FVector(-TNumericLimits<float>::Max());
    
    for(UObject* Object : Objects)
        if(AFGBuildable* Buildable = Cast<AFGBuildable>(Object))
        {
            if(UShapeComponent* Clearance = Buildable->GetClearanceComponent())
            {
                if(UBoxComponent* Box = Cast<UBoxComponent>(Clearance))
                {
                    const FVector Extents = Box->GetScaledBoxExtent();
                    for(int i = 0; i < (1 << 3); i++)
                    {
                        const int X = (i & 1) ? 1 : -1;
                        const int Y = (i & 2) ? 1 : -1;
                        const int Z = (i & 4) ? 1 : -1;
                        FVector Corner = FVector(Extents.X * X, Extents.Y * Y, Extents.Z * Z);
                        Min = Min.ComponentMin(Rotation.UnrotateVector(Buildable->GetActorRotation().RotateVector(Box->GetComponentTransform().GetLocation() + Corner - Buildable->GetActorLocation()) + Buildable->GetActorLocation()));
                        Max = Max.ComponentMax(Rotation.UnrotateVector(Buildable->GetActorRotation().RotateVector(Box->GetComponentTransform().GetLocation() + Corner - Buildable->GetActorLocation()) + Buildable->GetActorLocation()));
                    }
                }
                else
                {
                    // Are there any other types used as clearance?
                }
            }
            else
            {
                FActorSpawnParameters Params;
                Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                Params.bDeferConstruction = true;
                FTransform TempBuildingTransform = FTransform(FQuat::Identity, FVector::ZeroVector, Buildable->GetActorScale3D());
                AFGBuildable* TempBuilding = Buildable->GetWorld()->SpawnActor<AFGBuildable>(Buildable->GetClass(), TempBuildingTransform, Params);
                TempBuilding->bDeferBeginPlay = true;
                TempBuilding->FinishSpawning(TempBuildingTransform, true);
                FVector Origin;
                FVector Extents;
                TempBuilding->GetActorBounds(true, Origin, Extents);
                Extents = FVector(FGenericPlatformMath::RoundToFloat(Extents.X), FGenericPlatformMath::RoundToFloat(Extents.Y), FGenericPlatformMath::RoundToFloat(Extents.Z));

                for(int i = 0; i < (1 << 3); i++)
                {
                    const int X = (i & 1) ? 1 : -1;
                    const int Y = (i & 2) ? 1 : -1;
                    const int Z = (i & 4) ? 1 : -1;
                    FVector Corner = FVector(Extents.X * X, Extents.Y * Y, Extents.Z * Z);
                    Min = Min.ComponentMin(Rotation.UnrotateVector(Buildable->GetActorLocation() + Buildable->GetActorRotation().RotateVector(Origin + Corner)));
                    Max = Max.ComponentMax(Rotation.UnrotateVector(Buildable->GetActorLocation() + Buildable->GetActorRotation().RotateVector(Origin + Corner)));
                }
                TempBuilding->Destroy();
            }
        }

    Min = Rotation.RotateVector(Min);
    Max = Rotation.RotateVector(Max);

    const FVector Center = (Min + Max) / 2;

    const FVector Bounds = Rotation.UnrotateVector(Max - Center);
            
    return FAARotatedBoundingBox{Center, FVector(FGenericPlatformMath::RoundToFloat(Bounds.X), FGenericPlatformMath::RoundToFloat(Bounds.Y), FGenericPlatformMath::RoundToFloat(Bounds.Z)), Rotation};
}

TMap<TSubclassOf<UFGItemDescriptor>, int32> FAABuildingsDataHelper::CalculateBuildCosts(const TArray<UObject*>& Objects)
{
	TMap<TSubclassOf<UFGItemDescriptor>, int32> RequiredItems;
	for(UObject* Object : Objects)
	{
		if(AFGBuildable* Buildable = Cast<AFGBuildable>(Object))
		{
			{
				TArray<FItemAmount> BuildingIngredients = UFGRecipe::GetIngredients(Buildable->GetBuiltWithRecipe());
				for(const FItemAmount ItemAmount : BuildingIngredients)
					RequiredItems.FindOrAdd(ItemAmount.ItemClass) += ItemAmount.Amount;
			}
		}
	}
	return RequiredItems;
}

TMap<TSubclassOf<UFGItemDescriptor>, int32> FAABuildingsDataHelper::CalculateOtherItems(const TArray<UObject*>& Objects)
{
	TMap<TSubclassOf<UFGItemDescriptor>, int32> RequiredItems;
	for(UObject* Object : Objects)
	{
		if(AFGBuildable* Buildable = Cast<AFGBuildable>(Object))
		{
			{
				if(AFGBuildableFactory* FactoryBuildable = Cast<AFGBuildableFactory>(Buildable))
				{
					if(FactoryBuildable->mInventoryPotential)
					{
						TArray<FInventoryStack> Stacks;
						FactoryBuildable->mInventoryPotential->GetInventoryStacks(Stacks);
						for(const FInventoryStack Stack : Stacks)
							if(Stack.HasItems())
								RequiredItems.FindOrAdd(Stack.Item.ItemClass) += Stack.NumItems;
					}
				}
			}
		}
	}
	return RequiredItems;
}