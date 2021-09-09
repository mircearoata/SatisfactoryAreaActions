// Fill out your copyright notice in the Description page of Project Settings.

#include "AABlueprint.h"

#include "AABuildingsDataHelper.h"
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
	Ar << Header.GameVersion;
	Ar << Header.BlueprintName;
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

UAABlueprint* UAABlueprint::FromRootSet(UObject* WorldContext, TArray<AActor*>& Actors, TArray<AActor*>& OutActorsWithIssues)
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

	TArray<UObject*> ObjectsToSerialize;
	ObjectsToSerialize.Reserve(AllObjects.Num());
    
	FTopologicalSort::TopologicalSort(DependencyGraph, ObjectsToSerialize);
	bool Valid = ValidateObjects(ObjectsToSerialize, OutActorsWithIssues);
	if(!Valid)
		return nullptr;

	UAABlueprint* Blueprint = NewObject<UAABlueprint>(WorldContext->GetWorld());
	Blueprint->ObjectsToSerialize = ObjectsToSerialize;
	Blueprint->BlueprintHeader.GameVersion = FEngineVersion::Current().GetChangelist();
	Blueprint->BlueprintHeader.BoundingBox = FAABuildingsDataHelper::CalculateBoundingBox(ObjectsToSerialize);
	Blueprint->BlueprintHeader.BuildCosts = FAABuildingsDataHelper::CalculateBuildCosts(ObjectsToSerialize);
	Blueprint->BlueprintHeader.OtherItems = FAABuildingsDataHelper::CalculateOtherItems(ObjectsToSerialize);
	Blueprint->SerializeObjects();
	
	// Reset the Center and Rotation as the TOC is serialized relative to those
	Blueprint->BlueprintHeader.BoundingBox.Center = FVector::ZeroVector;
	Blueprint->BlueprintHeader.BoundingBox.Rotation = FRotator::ZeroRotator;

	return Blueprint;
}

void UAABlueprint::SerializeTOC()
{
	FAARotatedBoundingBox BoundingBox = BlueprintHeader.BoundingBox;
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