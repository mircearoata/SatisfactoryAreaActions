// Fill out your copyright notice in the Description page of Project Settings.

#include "AABlueprint.h"

#include "UObject/Object.h"
#include "AAObjectCollectorArchive.h"
#include "AAObjectReferenceArchive.h"
#include "AAObjectValidatorArchive.h"
#include "FGSaveInterface.h"
#include "FGSaveSession.h"
#include "Buildables/FGBuildable.h"
#include "Buildables/FGBuildableFactory.h"
#include "Components/SplineMeshComponent.h"
#include "TopologicalSort/DirectedGraph.h"
#include "TopologicalSort/TopologicalSort.h"


FORCEINLINE void PreSaveGame(UObject* Object)
{
	if (Object->Implements<UFGSaveInterface>())
		IFGSaveInterface::Execute_PreSaveGame(
			Object, UFGSaveSession::GetVersion(UFGSaveSession::Get(Object->GetWorld())->mSaveHeader),
			FEngineVersion::Current().GetChangelist());
}

FORCEINLINE void PostSaveGame(UObject* Object)
{
	if (Object->Implements<UFGSaveInterface>())
		IFGSaveInterface::Execute_PostSaveGame(
			Object, UFGSaveSession::GetVersion(UFGSaveSession::Get(Object->GetWorld())->mSaveHeader),
			FEngineVersion::Current().GetChangelist());
}

FORCEINLINE void PreLoadGame(UObject* Object)
{
	if (Object->Implements<UFGSaveInterface>())
		IFGSaveInterface::Execute_PreLoadGame(
			Object, UFGSaveSession::GetVersion(UFGSaveSession::Get(Object->GetWorld())->mSaveHeader),
			FEngineVersion::Current().GetChangelist());
}

FORCEINLINE void PostLoadGame(UObject* Object)
{
	if (Object->Implements<UFGSaveInterface>())
		IFGSaveInterface::Execute_PostLoadGame(
			Object, UFGSaveSession::GetVersion(UFGSaveSession::Get(Object->GetWorld())->mSaveHeader),
			FEngineVersion::Current().GetChangelist());
}

using HeaderFormatVersion = FAABlueprintHeader::HeaderFormatVersion;
using BlueprintFormatVersion = UAABlueprint::BlueprintFormatVersion;

FArchive& operator<< (FArchive& Ar, FAABlueprintHeader& Header)
{
	HeaderFormatVersion HeaderVersion;
	if(Ar.IsLoading())
		Ar << HeaderVersion;
	else
	{
		HeaderVersion = HeaderFormatVersion::LatestVersion;
		Ar << HeaderVersion;
	}
	if(HeaderVersion >= HeaderFormatVersion::InitialVersion)
		Ar << Header.GameVersion;
	Ar << Header.BoundingBox;
	Ar << Header.BuildCosts;
	Ar << Header.OtherItems;
	return Ar;
}

bool ValidateObjects(TArray<UObject*> Objects, TArray<AActor*>& OutActorsWithIssues)
{
	FAAObjectValidatorArchive ObjectValidator = FAAObjectValidatorArchive(Objects);
	for (UObject* Object :Objects)
		if (!ObjectValidator.Validate(Object))
		{
			UObject* ParentActor = Object;
			while(ParentActor && !ParentActor->IsA<AActor>())
				ParentActor = ParentActor->GetOuter();
			if(ParentActor)
				OutActorsWithIssues.Add(static_cast<AActor*>(ParentActor));
		}
	return OutActorsWithIssues.Num() == 0;
}

bool UAABlueprint::SetRootSet(TArray<AActor*>& Actors, TArray<AActor*> OutActorsWithIssues)
{
	TArray<UObject*> Objects;
	for(AActor* Building : Actors)
		Objects.Add(Building);
	TArray<UObject*> AllObjects;
	FAAObjectCollectorArchive ObjectCollector(&AllObjects);
	ObjectCollector.GetAllObjects(Objects);

	TDirectedGraph<UObject*> DependencyGraph;
	for(UObject* Object : AllObjects)
		DependencyGraph.AddNode(Object);
	for(UObject* Object : AllObjects)
	{
		if(DependencyGraph.Graph.Contains(Object->GetOuter()) && DependencyGraph.Graph.Contains(Object))
			DependencyGraph.AddEdge(Object->GetOuter(), Object);
		TArray<UObject*> Dependencies;
		IFGSaveInterface::Execute_GatherDependencies(Object, Dependencies);
		for(UObject* Dependency : Dependencies)
		{
			if(DependencyGraph.Graph.Contains(Object->GetOuter()) && DependencyGraph.Graph.Contains(Object))
				DependencyGraph.AddEdge(Dependency, Object);
		}
	}

	ObjectsToSerialize.Reserve(AllObjects.Num());
    
	FTopologicalSort::TopologicalSort(DependencyGraph, ObjectsToSerialize);
	return ValidateObjects(ObjectsToSerialize, OutActorsWithIssues);
}

FAARotatedBoundingBox UAABlueprint::CalculateBoundingBox()
{
    TMap<float, uint32> RotationCount;
    for(UObject* Object : ObjectsToSerialize)
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
    
    for(UObject* Object : ObjectsToSerialize)
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
                AFGBuildable* TempBuilding = this->GetWorld()->SpawnActor<AFGBuildable>(Buildable->GetClass(), TempBuildingTransform, Params);
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

TMap<TSubclassOf<UFGItemDescriptor>, int32> UAABlueprint::CalculateBuildCosts()
{
	TMap<TSubclassOf<UFGItemDescriptor>, int32> RequiredItems;
	for(UObject* Object : ObjectsToSerialize)
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

TMap<TSubclassOf<UFGItemDescriptor>, int32> UAABlueprint::CalculateOtherItems()
{
	TMap<TSubclassOf<UFGItemDescriptor>, int32> RequiredItems;
	for(UObject* Object : ObjectsToSerialize)
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

void UAABlueprint::SerializeTOC()
{
	FAARotatedBoundingBox BoundingBox = CalculateBoundingBox();
	FTransform Center = FTransform(FQuat(BoundingBox.Rotation), BoundingBox.Center, FVector::OneVector);
	FArchive Unused;
	FAAObjectReferenceArchive ObjectReferenceArchive(Unused, ObjectsToSerialize);
	for(UObject* Object : ObjectsToSerialize)
	{
		FAABlueprintObjectTOC TOC;
		TOC.Class = Object->GetClass();
		TOC.Name = Object->GetFName();
		TOC.Flags = Object->GetFlags();
		if(AActor* Actor = Cast<AActor>(Object))
		{
			TOC.HasTransform = true;
			TOC.Transform = Actor->GetTransform().GetRelativeTransform(Center);
			FAAObjectReference Owner;
			ObjectReferenceArchive.GetReferenceData(Actor, Owner.Idx, Owner.RelativePath);
			TOC.Owner = Owner;
		}
		else
			TOC.HasTransform = false;
		if(AFGBuildable* Buildable = Cast<AFGBuildable>(Object))
			TOC.BuiltWithRecipe = Buildable->GetBuiltWithRecipe();
		{
			FAAObjectReference Outer;
			ObjectReferenceArchive.GetReferenceData(Object->GetOuter(), Outer.Idx, Outer.RelativePath);
			TOC.Outer = Outer;
		}
		ObjectTOC.Add(TOC);
	}
}

void UAABlueprint::SerializeData()
{
	FMemoryWriter DataWriter = FMemoryWriter(ObjectsData.SerializedData, true);
	FAAObjectReferenceArchive DataAr(DataWriter, ObjectsToSerialize);
	DataAr.SetIsLoading(false);
	DataAr.SetIsSaving(true);
    
	for(UObject* Object : ObjectsToSerialize)
		Object->Serialize(DataAr);
}

void UAABlueprint::SerializeObjects()
{
	for (UObject* Object : ObjectsToSerialize)
		PreSaveGame(Object);

	SerializeTOC();
	SerializeData();
    
	for (UObject* Object : ObjectsToSerialize)
		PostSaveGame(Object);
}

void UAABlueprint::SerializeBlueprint(FArchive& Ar)
{
	Ar << BlueprintHeader;
	Ar << ObjectTOC;
	Ar << ObjectsData;
}

bool UAABlueprint::SaveBlueprint(const FString& FilePath)
{
	SerializeObjects();
	TArray<uint8> FileRaw;
	FMemoryWriter MemoryWriter(FileRaw);
	FObjectAndNameAsStringProxyArchive MemoryWriterProxy(MemoryWriter, true);
	BlueprintHeader.GameVersion = FEngineVersion::Current().GetChangelist();
	BlueprintHeader.BoundingBox = CalculateBoundingBox();
	// Reset the Center and Rotation as the TOC is serialized relative to those
	BlueprintHeader.BoundingBox.Center = FVector::ZeroVector;
	BlueprintHeader.BoundingBox.Rotation = FRotator::ZeroRotator;
	BlueprintHeader.BuildCosts = CalculateBuildCosts();
	BlueprintHeader.OtherItems = CalculateOtherItems();
	SerializeBlueprint(MemoryWriterProxy);
	return !MemoryWriter.IsError() && FFileHelper::SaveArrayToFile(FileRaw, *FilePath);
}

bool UAABlueprint::LoadBlueprint(const FString& FilePath)
{
	TArray<uint8> FileRaw;
	if (!FFileHelper::LoadFileToArray(FileRaw, *FilePath))
		return false;
	FMemoryReader MemoryReader(FileRaw);
	FObjectAndNameAsStringProxyArchive MemoryReaderProxy(MemoryReader, true);
	SerializeBlueprint(MemoryReaderProxy);
	return !MemoryReader.IsError();
}

FArchive& operator<<(FArchive& Ar, FAAObjectReference& ObjectReference)
{
	Ar << ObjectReference.Idx;
	Ar << ObjectReference.RelativePath;
	return Ar;
}

FArchive& operator<<(FArchive& Ar, FAABlueprintObjectTOC& BuildingTOC)
{
	Ar << BuildingTOC.Class;
	Ar << BuildingTOC.Name;
	if(Ar.IsLoading())
	{
		uint32 FlagsInt = static_cast<uint32>(BuildingTOC.Flags);
		Ar << FlagsInt;
	}
	else
	{
		uint32 FlagsInt;
		Ar << FlagsInt;
		BuildingTOC.Flags = static_cast<EObjectFlags>(FlagsInt);
	}
	Ar << BuildingTOC.HasTransform;
	if (BuildingTOC.HasTransform)
		Ar << BuildingTOC.Transform;
	Ar << BuildingTOC.BuiltWithRecipe;
	Ar << BuildingTOC.Owner;
	Ar << BuildingTOC.Outer;
	return Ar;
}

FArchive& operator<<(FArchive& Ar, FAABlueprintObjectsData& BuildingData)
{
	Ar << BuildingData.SerializedData;
	return Ar;
}

FString UAABlueprint::GetBlueprintPath(FString Name)
{
	return FPaths::ProjectSavedDir() / TEXT("AreaActionsBlueprints") / FString::Printf(TEXT("%s.aabp"), *Name);
}