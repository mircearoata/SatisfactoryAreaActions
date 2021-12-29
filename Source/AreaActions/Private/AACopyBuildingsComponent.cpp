// Fill out your copyright notice in the Description page of Project Settings.

#include "AACopyBuildingsComponent.h"

#include "AABlueprintFunctionLibrary.h"
#include "AABuildingsDataHelper.h"
#include "AAObjectCollectorArchive.h"
#include "AAObjectReferenceArchive.h"
#include "AAObjectValidatorArchive.h"
#include "AASerializationHelpers.h"
#include "Buildables/FGBuildableConveyorBase.h"
#include "FGColoredInstanceMeshProxy.h"
#include "FGFactorySettings.h"
#include "GameFramework/Actor.h"
#include "FGCircuitConnectionComponent.h"
#include "FGCircuitSubsystem.h"
#include "FGPipeConnectionComponent.h"
#include "TopologicalSort/TopologicalSort.h"
#include "FGGameState.h"
#include "FGProductionIndicatorInstanceComponent.h"

UAACopyBuildingsComponent::UAACopyBuildingsComponent()
{
    ValidCheckSkipProperties.Add(AActor::StaticClass()->FindPropertyByName(TEXT("Owner")));
}

bool UAACopyBuildingsComponent::SetActors(const TArray<AActor*>& Actors, TArray<AFGBuildable*>& OutBuildingsWithIssues, FText& Error)
{
    TArray<AFGBuildable*> Buildings;
    for (AActor* Actor : Actors)
        if (Actor->IsA<AFGBuildable>())
            Buildings.Add(static_cast<AFGBuildable*>(Actor));
    return SetBuildings(Buildings, OutBuildingsWithIssues, Error);
}


void SafeAddEdge(TDirectedGraph<UObject*>& DependencyGraph, UObject* From, UObject* To)
{
    // DependencyGraph.addNode(From);
    // DependencyGraph.addNode(To);
    if(DependencyGraph.Graph.Contains(From) && DependencyGraph.Graph.Contains(To))
        DependencyGraph.AddEdge(From, To);
}

bool UAACopyBuildingsComponent::SetBuildings(const TArray<AFGBuildable*>& Buildings,
                                             TArray<AFGBuildable*>& OutBuildingsWithIssues,
                                             FText& Error)
{
    if(Buildings.Num() == 0)
    {
        Error = FText::FromString(TEXT("No buildings in the area."));
        return false;
    }
    TArray<UObject*> Objects;
    for(AFGBuildable* Building : Buildings)
        Objects.Add(Building);
	TArray<UObject*> AllObjects;
    FAAObjectCollectorArchive ObjectCollector(&AllObjects);
    ObjectCollector.GetAllObjects(Objects);

    TDirectedGraph<UObject*> DependencyGraph;
    for(UObject* Object : AllObjects)
        DependencyGraph.AddNode(Object);
    for(UObject* Object : AllObjects)
    {
        SafeAddEdge(DependencyGraph, Object->GetOuter(), Object);
        TArray<UObject*> Dependencies;
        IFGSaveInterface::Execute_GatherDependencies(Object, Dependencies);
        for(UObject* Dependency : Dependencies)
        {
            SafeAddEdge(DependencyGraph, Dependency, Object);
            // SML::Logging::warning(*Dependency->GetFullName(), " is a dependency of ", *Object->GetFullName());
        }
    }

    this->Original.Reserve(AllObjects.Num());
    
    FTopologicalSort::TopologicalSort(DependencyGraph, this->Original);
    const bool Ret = ValidateObjects(OutBuildingsWithIssues);
    if(Ret)
    {
        CalculateBounds();
        SerializeOriginal();
    }
    return Ret;
}

void UAACopyBuildingsComponent::CalculateBounds()
{
    this->BuildingsBounds = FAABuildingsDataHelper::CalculateBoundingBox(this->Original);
}

void UAACopyBuildingsComponent::SerializeOriginal()
{
    for(UObject* Object : Original)
        UAASerializationHelpers::CallPreSaveGame(Object);
    
    FMemoryWriter Writer = FMemoryWriter(Serialized, true);
    FAAObjectReferenceArchive Ar(Writer, this->Original);
    Ar.SetIsLoading(false);
    Ar.SetIsSaving(true);
    
    for(UObject* Object : this->Original)
        Object->Serialize(Ar);
    
    for(UObject* Object : Original)
        UAASerializationHelpers::CallPostSaveGame(Object);
}

bool UAACopyBuildingsComponent::ValidateObjects(TArray<AFGBuildable*>& OutBuildingsWithIssues)
{
    FAAObjectValidatorArchive ObjectValidator = FAAObjectValidatorArchive(this->Original);
    for (UObject* Object : this->Original)
        if (!ObjectValidator.Validate(Object))
        {
            UObject* ParentBuilding = Object;
            while(ParentBuilding && !ParentBuilding->IsA<AFGBuildable>())
                ParentBuilding = ParentBuilding->GetOuter();
            if(ParentBuilding)
                OutBuildingsWithIssues.Add(static_cast<AFGBuildable*>(ParentBuilding));
        }
    return OutBuildingsWithIssues.Num() == 0;
}

FTransform UAACopyBuildingsComponent::TransformAroundPoint(const FTransform OriginalTransform, const FVector Offset, const FRotator Rotation, const FVector RotationCenter)
{
    const FVector NewLocation = Rotation.RotateVector(OriginalTransform.GetLocation() - RotationCenter) + RotationCenter + Offset;
    const FQuat NewRotation = Rotation.Quaternion() * OriginalTransform.GetRotation();
    return FTransform(NewRotation, NewLocation, OriginalTransform.GetScale3D());
}

void UAACopyBuildingsComponent::FixReferencesForCopy(const FCopyMap& Copy)
{
    TArray<UObject*> PreviewObjects;
    for(UObject* Object : this->Original)
    {
        UObject* NewObject = Copy.GetObject(Object);
        UAASerializationHelpers::CallPreLoadGame(NewObject);
        PreviewObjects.Add(NewObject);
    }
    
    FMemoryReader Reader = FMemoryReader(Serialized, true);
    FAAObjectReferenceArchive Ar2(Reader, PreviewObjects);
    Ar2.SetIsLoading(true);
    Ar2.SetIsSaving(false);
    for(UObject* Object : this->Original)
    {
        UObject* NewObject = Copy.GetObject(Object);
        NewObject->Serialize(Ar2);
    }
    
    for(UObject* Object : this->Original)
    {
        UObject* NewObject = Copy.GetObject(Object);
        UAASerializationHelpers::CallPostLoadGame(NewObject);
    }

    // Remove items
    {
        for(UObject* Object : this->Original)
        {
            if(AFGBuildable* Buildable = Cast<AFGBuildable>(Copy.GetObject(Object)))
            {
                {
                    // Remove items from inventories
                    TArray<UFGInventoryComponent*> BuildingInventories;
                    Buildable->GetComponents<UFGInventoryComponent>(BuildingInventories);
                
                    if(AFGBuildableFactory* FactoryBuildable = Cast<AFGBuildableFactory>(Buildable))
                    {
                        BuildingInventories.Remove(FactoryBuildable->mInventoryPotential); // Keep potential inventory
                    }
                
                    for(UFGInventoryComponent* InventoryComponent : BuildingInventories)
                    {
                        for(int i = 0; i < InventoryComponent->GetSizeLinear(); i++)
                            if(InventoryComponent->IsSomethingOnIndex(i))
                                InventoryComponent->RemoveAllFromIndex(i);
                    }
                }
                {
                    // Remove items from conveyors
                    if(AFGBuildableConveyorBase* ConveyorBase = Cast<AFGBuildableConveyorBase>(Buildable))
                    {
                        for(int i = ConveyorBase->mItems.Num() - 1; i >= 0; i--)
                            if(ConveyorBase->mItems.IsValidIndex(i))
                                ConveyorBase->Factory_RemoveItemAt(i);
                    }
                }
            }
        }
    }
    
    // Fixes for stuff that doesn't cause issues, but is nice to have
    {
        // Fix circuits
        AFGCircuitSubsystem* CircuitSubsystem = AFGCircuitSubsystem::Get(GetWorld());
        for(int i = 0; i < PreviewObjects.Num(); i++)
        {
            if(UFGCircuitConnectionComponent* Connection = Cast<UFGCircuitConnectionComponent>(PreviewObjects[i]))
            {
                if(Connection->GetCircuitID() == -1)
                {
                    TArray<UFGCircuitConnectionComponent*> Connections;
                    Connection->GetConnections(Connections);
                    Connection->GetHiddenConnections(Connections);
                    if(Connections.Num() > 0)
                        CircuitSubsystem->ConnectComponents(Connection, Connections[0]); // This will rebuild the entire circuit
                }
            }
        }
    }
    // Fix pipes
    {
        for(int i = 0; i < PreviewObjects.Num(); i++)
        {
            if(UFGPipeConnectionComponent* Connection = Cast<UFGPipeConnectionComponent>(PreviewObjects[i]))
            {
                {
                    UFGPipeConnectionComponentBase* Other = Connection->GetConnection();
                    if(Other)
                    {
                        Connection->ClearConnection();
                        Connection->SetConnection(Other);
                    }
                }
            }
        }
    }
}

int UAACopyBuildingsComponent::AddCopy(const FVector Offset, const FRotator Rotation, const FVector RotationCenter, const bool Relative)
{
    this->CopyLocations.Add(CurrentId, FCopyLocation(Offset, Rotation, RotationCenter, Relative));
    this->Preview.Add(CurrentId);
    for(UObject* Object : this->Original)
    {
        if(AFGBuildable* Buildable = Cast<AFGBuildable>(Object))
        {
            if(Buildable->GetBuiltWithRecipe())
            {
                AFGBuildableHologram* Hologram = static_cast<AFGBuildableHologram*>(AFGHologram::SpawnHologramFromRecipe(Buildable->GetBuiltWithRecipe(), GetOwner(), FVector::ZeroVector));
                FTransform NewTransform = TransformAroundPoint(Buildable->GetActorTransform(), Relative ? BuildingsBounds.Rotation.RotateVector(Offset) : Offset, Rotation, RotationCenter);
                Hologram->SetActorTransform(NewTransform);
                Hologram->SetActorHiddenInGame(false);
                Hologram->SetPlacementMaterialState(EHologramMaterialState::HMS_OK);
                this->Preview[CurrentId].Holograms.Add(Buildable, Hologram);
            }
        }
    }
    return CurrentId++;
}

void UAACopyBuildingsComponent::MoveCopy(const int CopyId, const FVector Offset, const FRotator Rotation, const FVector RotationCenter, const bool Relative)
{
    FCopyLocation NewLocation = FCopyLocation(Offset, Rotation, RotationCenter, Relative);
    if(this->CopyLocations[CopyId] == NewLocation)
        return;
    
    this->CopyLocations[CopyId] = NewLocation;
    for(const auto& [Buildable, Hologram] : this->Preview[CopyId].Holograms)
    {
        FTransform NewTransform = TransformAroundPoint(Buildable->GetActorTransform(), Relative ? BuildingsBounds.Rotation.RotateVector(Offset) : Offset, Rotation, RotationCenter);
        Hologram->SetActorTransform(NewTransform);
    }
}

void UAACopyBuildingsComponent::RemoveCopy(const int CopyId)
{
    if(!Preview.Contains(CopyId))
        return;
    for(const auto& [_, Hologram] : this->Preview[CopyId].Holograms)
        Hologram->Destroy();
    this->Preview.Remove(CopyId);
    this->CopyLocations.Remove(CopyId);
}

bool UAACopyBuildingsComponent::Finish(const TArray<UFGInventoryComponent*> Inventories, TMap<TSubclassOf<UFGItemDescriptor>, int32>& OutMissingItems)
{
    if(this->CopyLocations.Num() == 0)
        return true;

    const bool UseBuildCosts = !static_cast<AFGGameState*>(GetWorld()->GetGameState())->GetCheatNoCost();
    if(UseBuildCosts && !UAABlueprintFunctionLibrary::TakeItemsFromInventories(Inventories, GetRequiredItems(), OutMissingItems))
        return false;

    for(const auto& [CopyId, CopyLocation] : this->CopyLocations)
    {
        FCopyMap CurrentCopy;
        for(UObject* Object : this->Original)
        {
            UObject* NewObj;
            if(AActor* Actor = Cast<AActor>(Object))
            {
                FTransform NewTransform = TransformAroundPoint(Actor->GetActorTransform(), CopyLocation.Relative ? BuildingsBounds.Rotation.RotateVector(CopyLocation.Offset) : CopyLocation.Offset, CopyLocation.Rotation, CopyLocation.RotationCenter);
                FActorSpawnParameters Params;
                Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                Params.bDeferConstruction = true;
                Params.Owner = Actor->GetOwner();
                AActor* NewActor;
                if(AFGBuildable* Buildable = Cast<AFGBuildable>(Actor))
                    NewActor = AFGBuildableSubsystem::Get(GetWorld())->BeginSpawnBuildable(Buildable->GetClass(), NewTransform);
                else
                    NewActor = this->GetWorld()->SpawnActor<AActor>(Actor->GetClass(), NewTransform, Params);
                NewActor->bDeferBeginPlay = true;
                NewActor->FinishSpawning(FTransform::Identity, true);
                NewObj = NewActor;
            }
            else
            {
                UObject* Outer = CurrentCopy.GetObjectChecked(Object->GetOuter());
                if(Outer != Object->GetOuter())
                    NewObj = FindObject<UObject>(CurrentCopy.GetObjectChecked(Object->GetOuter()), *Object->GetName());
                else
                    NewObj = NewObject<UObject>(CurrentCopy.GetObjectChecked(Object->GetOuter()), Object->GetClass(), NAME_None, Object->GetFlags());
                if(!NewObj)
                    NewObj = NewObject<UObject>(CurrentCopy.GetObjectChecked(Object->GetOuter()), Object->GetClass(), FName(*Object->GetName()), Object->GetFlags());
            }
            CurrentCopy.AddObject(Object, NewObj);
        }
    
        this->FixReferencesForCopy(CurrentCopy);
    
        for(UObject* Object : this->Original)
        {
            UObject* NewObject = CurrentCopy.GetObject(Object);
            if(AActor* NewActor = Cast<AActor>(NewObject))
            {
                NewActor->DeferredBeginPlay();
            }
        }

        for(const auto& [_, Hologram] : this->Preview[CopyId].Holograms)
            Hologram->Destroy();
        this->Preview.Remove(CopyId);
    }
    this->CopyLocations.Empty();
    return true;
}

AFGBuildableHologram* UAACopyBuildingsComponent::GetPreviewHologram(int CopyId, AFGBuildable* Buildable)
{
    if(!this->Preview.Contains(CopyId)) return nullptr;
    if(!this->Preview[CopyId].Holograms.Contains(Buildable)) return nullptr;
    return this->Preview[CopyId].Holograms[Buildable];
}

void UAACopyBuildingsComponent::GetAllPreviewHolograms(TArray<AFGBuildableHologram*>& OutPreviewHolograms)
{
    for(const auto& [CopyId, CopyPreview] : this->Preview)
    {
        for(const auto& [_, Hologram] : CopyPreview.Holograms)
        {
            OutPreviewHolograms.Add(Hologram);
        }
    }
}

void UAACopyBuildingsComponent::GetBuildingHolograms(const int CopyId, TMap<AFGBuildable*, AFGBuildableHologram*>& OutPreviewHolograms)
{
    OutPreviewHolograms = this->Preview[CopyId].Holograms;
}

TMap<TSubclassOf<UFGItemDescriptor>, int32> UAACopyBuildingsComponent::GetRequiredItems()
{
    TMap<TSubclassOf<UFGItemDescriptor>, int32> ItemsPerCopy;
    for(UObject* Object : this->Original)
    {
        if(AFGBuildable* Buildable = Cast<AFGBuildable>(Object))
        {
            {
                TArray<FItemAmount> BuildingIngredients = UFGRecipe::GetIngredients(Buildable->GetBuiltWithRecipe());
                for(const FItemAmount ItemAmount : BuildingIngredients)
                    ItemsPerCopy.FindOrAdd(ItemAmount.ItemClass) += ItemAmount.Amount;
            }
            {
                if(AFGBuildableFactory* FactoryBuildable = Cast<AFGBuildableFactory>(Buildable))
                {
                    if(FactoryBuildable->mInventoryPotential)
                    {
                        TArray<FInventoryStack> Stacks;
                        FactoryBuildable->mInventoryPotential->GetInventoryStacks(Stacks);
                        for(const FInventoryStack Stack : Stacks)
                            if(Stack.HasItems())
                                ItemsPerCopy.FindOrAdd(Stack.Item.ItemClass) += Stack.NumItems;
                    }
                }
            }
        }
    }
    
    TMap<TSubclassOf<UFGItemDescriptor>, int32> TotalItems;
    for(auto ItemAmount : ItemsPerCopy)
        TotalItems.Add(ItemAmount.Key, ItemAmount.Value * this->Preview.Num());
    
    return TotalItems;
}

void UAACopyBuildingsComponent::SetHologramState(EHologramMaterialState NewState)
{
    for(const auto& [CopyId, CopyPreview] : this->Preview)
    {
        for(const auto& [_, Hologram] : CopyPreview.Holograms)
        {
            Hologram->SetPlacementMaterialState(NewState);
        }
    }
}
