// Fill out your copyright notice in the Description page of Project Settings.

#include "AACopyBuildingsComponent.h"


#include "AAObjectCollectorArchive.h"
#include "AAObjectReferenceArchive.h"
#include "AAObjectValidatorArchive.h"
#include "FGBuildableManufacturer.h"
#include "FGBuildableConveyorBase.h"
#include "SML/util/Logging.h"
#include "FGColoredInstanceMeshProxy.h"
#include "FGFactorySettings.h"
#include "GameFramework/Actor.h"
#include "util/TopologicalSort.h"
#include "FGCircuitConnectionComponent.h"
#include "FGCircuitSubsystem.h"
#include "FGPipeConnectionComponent.h"

#pragma optimize("", off)

void PreSaveGame(UObject* Object)
{
    if (Object->Implements<UFGSaveInterface>())
        IFGSaveInterface::Execute_PreSaveGame(
            Object, UFGSaveSession::GetVersion(UFGSaveSession::Get(Object->GetWorld())->mSaveHeader),
            FEngineVersion::Current().GetChangelist());
}

void PostSaveGame(UObject* Object)
{
    if (Object->Implements<UFGSaveInterface>())
        IFGSaveInterface::Execute_PostSaveGame(
            Object, UFGSaveSession::GetVersion(UFGSaveSession::Get(Object->GetWorld())->mSaveHeader),
            FEngineVersion::Current().GetChangelist());
}

void PreLoadGame(UObject* Object)
{
    if (Object->Implements<UFGSaveInterface>())
        IFGSaveInterface::Execute_PreLoadGame(
            Object, UFGSaveSession::GetVersion(UFGSaveSession::Get(Object->GetWorld())->mSaveHeader),
            FEngineVersion::Current().GetChangelist());
}

void PostLoadGame(UObject* Object)
{
    if (Object->Implements<UFGSaveInterface>())
        IFGSaveInterface::Execute_PostLoadGame(
            Object, UFGSaveSession::GetVersion(UFGSaveSession::Get(Object->GetWorld())->mSaveHeader),
            FEngineVersion::Current().GetChangelist());
}

FVector FRotatedBoundingBox::GetCorner(const uint32 CornerNum) const
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

UAACopyBuildingsComponent::UAACopyBuildingsComponent()
{
    ValidCheckSkipProperties.Add(AActor::StaticClass()->FindPropertyByName(TEXT("Owner")));
}

bool UAACopyBuildingsComponent::SetActors(TArray<AActor*>& Actors, TArray<AFGBuildable*>& OutBuildingsWithIssues, FText& Error)
{
    TArray<AFGBuildable*> Buildings;
    for (AActor* Actor : Actors)
        if (Actor->IsA<AFGBuildable>())
            Buildings.Add(static_cast<AFGBuildable*>(Actor));
    return SetBuildings(Buildings, OutBuildingsWithIssues, Error);
}


void SafeAddEdge(SML::TopologicalSort::DirectedGraph<UObject*>& DependencyGraph, UObject* From, UObject* To)
{
    // DependencyGraph.addNode(From);
    // DependencyGraph.addNode(To);
    if(DependencyGraph.graph.Contains(From) && DependencyGraph.graph.Contains(To))
        DependencyGraph.addEdge(From, To);
}

bool UAACopyBuildingsComponent::SetBuildings(TArray<AFGBuildable*>& Buildings,
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
    FAAObjectCollectorArchive ObjectCollector(&this->Original);
    ObjectCollector.GetAllObjects(Objects);

    SML::TopologicalSort::DirectedGraph<UObject*> DependencyGraph;
    for(UObject* Object : this->Original)
        DependencyGraph.addNode(Object);
    for(UObject* Object : this->Original)
    {
        SafeAddEdge(DependencyGraph, Object->GetOuter(), Object);
        TArray<UObject*> Dependencies;
        IFGSaveInterface::Execute_GatherDependencies(Object, Dependencies);
        for(UObject* Dependency : Dependencies)
        {
            SafeAddEdge(DependencyGraph, Dependency, Object);
            SML::Logging::warning(*Dependency->GetFullName(), " is a dependency of ", *Object->GetFullName());
        }
    }
    
    this->Original = topologicalSort(DependencyGraph);
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
    TMap<float, uint32> RotationCount;
    for(UObject* Object : this->Original)
        if(AActor* Actor = Cast<AActor>(Object))
            RotationCount.FindOrAdd(FGenericPlatformMath::Fmod(FGenericPlatformMath::Fmod(Actor->GetActorRotation().Yaw, 90) + 90, 90))++;

    RotationCount.ValueSort([](const uint32& A, const uint32& B) {
        return A > B;
    });

    const FRotator Rotation = FRotator(0, (*RotationCount.CreateIterator()).Key, 0);

    FVector Min = FVector(TNumericLimits<float>::Max());
    FVector Max = FVector(-TNumericLimits<float>::Max());
    
    for(UObject* Object : this->Original)
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
            
    this->BuildingsBounds = FRotatedBoundingBox{Center, FVector(FGenericPlatformMath::RoundToFloat(Bounds.X), FGenericPlatformMath::RoundToFloat(Bounds.Y), FGenericPlatformMath::RoundToFloat(Bounds.Z)), Rotation};
}

void UAACopyBuildingsComponent::SerializeOriginal()
{
    for(UObject* Object : Original)
        PreSaveGame(Object);
    
    FMemoryWriter Writer = FMemoryWriter(Serialized, true);
    FAAObjectReferenceArchive Ar(Writer, this->Original);
    Ar.SetIsLoading(false);
    Ar.SetIsSaving(true);
    
    for(UObject* Object : this->Original)
        Object->Serialize(Ar);
    
    for(UObject* Object : Original)
        PostSaveGame(Object);
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

FTransform TransformAroundPoint(const FTransform Original, const FVector Offset, const FRotator Rotation, const FVector RotationCenter)
{
    const FVector NewLocation = Rotation.RotateVector(Original.GetLocation() - RotationCenter) + RotationCenter + Offset;
    const FQuat NewRotation = Rotation.Quaternion() * Original.GetRotation();
    return FTransform(NewRotation, NewLocation, Original.GetScale3D());
}

void UAACopyBuildingsComponent::FixReferencesForCopy(const int CopyId)
{
    TArray<UObject*> PreviewObjects;
    for(UObject* Object : this->Original)
    {
        UObject* NewObject = this->Preview[CopyId].GetObject(Object);
        PreLoadGame(NewObject);
        PreviewObjects.Add(NewObject);
    }
    
    FMemoryReader Reader = FMemoryReader(Serialized, true);
    FAAObjectReferenceArchive Ar2(Reader, PreviewObjects);
    Ar2.SetIsLoading(true);
    Ar2.SetIsSaving(false);
    for(UObject* Object : this->Original)
    {
        UObject* NewObject = this->Preview[CopyId].GetObject(Object);
        NewObject->Serialize(Ar2);
    }
    
    for(UObject* Object : this->Original)
    {
        UObject* NewObject = this->Preview[CopyId].GetObject(Object);
        PostLoadGame(NewObject);
    }

    // Remove items
    {
        for(UObject* Object : this->Original)
        {
            if(AFGBuildable* Buildable = Cast<AFGBuildable>(this->Preview[CopyId].GetObject(Object)))
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
                            InventoryComponent->RemoveAllFromIndex(i);
                    }
                }
                {
                    // Remove items from conveyors
                    if(AFGBuildableConveyorBase* ConveyorBase = Cast<AFGBuildableConveyorBase>(Buildable))
                    {
                        for(int i = ConveyorBase->mItems.Num() - 1; i >= 0; i--)
                            if(ConveyorBase->mItems.IsValidIndex(i))
                                ConveyorBase->Factory_RemoveItemAtAccessor(i);
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
    this->Preview.Add(CurrentId);
    for(UObject* Object : this->Original)
    {
        UObject* PreviewObject;
        if(AActor* Actor = Cast<AActor>(Object))
        {
            FTransform NewTransform = TransformAroundPoint(Actor->GetActorTransform(), Relative ? BuildingsBounds.Rotation.RotateVector(Offset) : Offset, Rotation, RotationCenter);
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
            PreviewObject = NewActor;
        }
        else
        {
            UObject* Outer = this->Preview[CurrentId].GetObjectChecked(Object->GetOuter());
            if(Outer != Object->GetOuter())
                PreviewObject = FindObject<UObject>(this->Preview[CurrentId].GetObjectChecked(Object->GetOuter()), *Object->GetName());
            else
                PreviewObject = NewObject<UObject>(this->Preview[CurrentId].GetObjectChecked(Object->GetOuter()), Object->GetClass(), NAME_None, Object->GetFlags());
            if(!PreviewObject)
                PreviewObject = NewObject<UObject>(this->Preview[CurrentId].GetObjectChecked(Object->GetOuter()), Object->GetClass(), FName(*Object->GetName()), Object->GetFlags());
        }
        this->Preview[CurrentId].AddObject(Object, PreviewObject);
    }
    
    this->FixReferencesForCopy(CurrentId);
    
    for(UObject* Object : this->Original)
    {
        UObject* NewObject = this->Preview[CurrentId].GetObject(Object);
        if(AActor* NewActor = Cast<AActor>(NewObject))
        {
            NewActor->DeferredBeginPlay();
            if(AFGBuildable* NewBuilding = Cast<AFGBuildable>(NewActor))
            {
                TArray<UFGColoredInstanceMeshProxy*> ColoredInstanceMeshProxies;
                NewBuilding->GetComponents<UFGColoredInstanceMeshProxy>(ColoredInstanceMeshProxies);
                for (UFGColoredInstanceMeshProxy* Mesh : ColoredInstanceMeshProxies)
                    Mesh->SetInstanced(false);
                if(ColoredInstanceMeshProxies.Num() > 0)
                {
                    UFGColoredInstanceMeshProxy* Mesh = ColoredInstanceMeshProxies[0];
                    for (int MatNum = 0; MatNum < Mesh->GetNumMaterials(); MatNum++)
                        Mesh->SetMaterial(MatNum, UFGFactorySettings::Get()->mDefaultValidPlacementMaterial);
                }
            }
        }
    }
    return CurrentId++;
}

void UAACopyBuildingsComponent::MoveCopy(const int CopyId, const FVector Offset, const FRotator Rotation, const FVector RotationCenter, const bool Relative)
{
    for(UObject* Object : this->Original)
    {
        UObject* NewObject = this->Preview[CopyId].GetObject(Object);
        if(AActor* NewActor = Cast<AActor>(NewObject))
        {
            AActor* Actor = static_cast<AActor*>(Object);
            FTransform NewTransform = TransformAroundPoint(Actor->GetActorTransform(), Relative ? BuildingsBounds.Rotation.RotateVector(Offset) : Offset, Rotation, RotationCenter);
            const EComponentMobility::Type CurrentMobility = NewActor->GetRootComponent()->Mobility;
            NewActor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
            NewActor->SetActorTransform(NewTransform);
            NewActor->GetRootComponent()->SetMobility(CurrentMobility);
        }
    }
}

void UAACopyBuildingsComponent::RemoveCopy(const int CopyId)
{
    for(UObject* Object : this->Original)
    {
        UObject* NewObject = this->Preview[CopyId].GetObject(Object);
        if(AActor* NewActor = Cast<AActor>(NewObject))
            NewActor->Destroy();
    }
    this->Preview.Remove(CopyId);
}

bool CheckItems(TMap<TSubclassOf<UFGItemDescriptor>, int32> RemainingItems, TArray<UFGInventoryComponent*> Inventories, TArray<FInventoryStack>& OutMissingItems, const bool TakeItems = false)
{
    if(TakeItems)
    {
        if(!CheckItems(RemainingItems, Inventories, OutMissingItems, false))
            return false;
    }
    
    for(UFGInventoryComponent* Inventory : Inventories)
    {
        TArray<FInventoryStack> Stacks;
        Inventory->GetInventoryStacks(Stacks);
        for(FInventoryStack& Stack : Stacks)
        {
            if(!Stack.HasItems()) continue;
            if(RemainingItems.Contains(Stack.Item.ItemClass))
            {
                const int TakenItems = FGenericPlatformMath::Min(Stack.NumItems, RemainingItems[Stack.Item.ItemClass]);
                RemainingItems[Stack.Item.ItemClass] -= TakenItems;
                if(RemainingItems[Stack.Item.ItemClass] == 0)
                    RemainingItems.Remove(Stack.Item.ItemClass);
                if(TakeItems)
                    Inventory->Remove(Stack.Item.ItemClass, TakenItems);
            }
        }
    }

    if(RemainingItems.Num() != 0)
    {
        for(const auto ItemAmount : RemainingItems)
            OutMissingItems.Add(FInventoryStack(ItemAmount.Value, ItemAmount.Key));
        return false;
    }
    return true;
}

bool UAACopyBuildingsComponent::TryTakeItems(TArray<UFGInventoryComponent*> Inventories, TArray<FInventoryStack>& OutMissingItems)
{
    if(this->Preview.Num() == 0)
        return true;
    
    TArray<int> CopyIds;
    this->Preview.GetKeys(CopyIds);
    const int FirstCopy = CopyIds[0];
    
    const bool UseBuildCosts = !static_cast<AFGGameState*>(GetWorld()->GetGameState())->GetCheatNoCost();
    TMap<TSubclassOf<UFGItemDescriptor>, int32> ItemsPerCopy;
    for(UObject* Object : this->Original)
    {
        if(AFGBuildable* Buildable = Cast<AFGBuildable>(Object))
        {
            {
                if(UseBuildCosts)
                {
                    TArray<FItemAmount> BuildingIngredients = UFGRecipe::GetIngredients(Buildable->GetBuiltWithRecipe());
                    for(const FItemAmount ItemAmount : BuildingIngredients)
                        ItemsPerCopy.FindOrAdd(ItemAmount.ItemClass) += ItemAmount.Amount;
                }
            }
            {
                TArray<UFGInventoryComponent*> BuildingInventories;
                this->Preview[FirstCopy].GetObject(Buildable)->GetComponents<UFGInventoryComponent>(BuildingInventories);
                for(UFGInventoryComponent* InventoryComponent : BuildingInventories)
                {
                    TArray<FInventoryStack> Stacks;
                    InventoryComponent->GetInventoryStacks(Stacks);
                    for(const FInventoryStack Stack : Stacks)
                        if(Stack.HasItems())
                            ItemsPerCopy.FindOrAdd(Stack.Item.ItemClass) += Stack.NumItems;
                }
            }
        }
    }
    
    TMap<TSubclassOf<UFGItemDescriptor>, int32> TotalItems;
    for(auto ItemAmount : ItemsPerCopy)
        TotalItems.Add(ItemAmount.Key, ItemAmount.Value * this->Preview.Num());

    return CheckItems(TotalItems, Inventories, OutMissingItems, true);
}

bool UAACopyBuildingsComponent::Finish(TArray<UFGInventoryComponent*> Inventories, TArray<FInventoryStack>& OutMissingItems)
{
    if(this->Preview.Num() == 0)
        return true;
    
    TArray<int> CopyIds;
    this->Preview.GetKeys(CopyIds);

    if(!TryTakeItems(Inventories, OutMissingItems))
        return false;
   
    for (int32 CopyId : CopyIds)
    {
        this->FixReferencesForCopy(CopyId);
        for(UObject* Object : this->Original)
        {
            UObject* NewObject = this->Preview[CopyId].GetObject(Object);
            if(AActor* NewActor = Cast<AActor>(NewObject))
            {
                if(AFGBuildable* NewBuilding = Cast<AFGBuildable>(NewActor))
                {
                    TArray<UFGColoredInstanceMeshProxy*> ColoredInstanceMeshProxies;
                    NewBuilding->GetComponents<UFGColoredInstanceMeshProxy>(ColoredInstanceMeshProxies);
                    for (UFGColoredInstanceMeshProxy* Mesh : ColoredInstanceMeshProxies)
                    {
                        Mesh->SetInstanced(true);
                        UFGColoredInstanceMeshProxy* OriginalMeshComponent = nullptr;
                        TArray<UFGColoredInstanceMeshProxy*> OriginalColoredInstanceMeshProxies;
                        NewBuilding->GetComponents<UFGColoredInstanceMeshProxy>(OriginalColoredInstanceMeshProxies);
                        for (UFGColoredInstanceMeshProxy* OriginalBuildingMesh : OriginalColoredInstanceMeshProxies)
                        {
                            if (OriginalBuildingMesh->GetName() == Mesh->GetName())
                            {
                                OriginalMeshComponent = OriginalBuildingMesh;
                                break;
                            }
                        }

                        if (!OriginalMeshComponent)
                        {
                            SML::Logging::fatal(*FString::Printf(
                                TEXT("Mesh component %s of %s does not exist. This shouldn't happen!"),
                                *Mesh->GetName(), *Object->GetName())); // I would like SML::Logging::wtf
                            continue;
                        }
                
                        for (int MatNum = 0; MatNum < Mesh->GetNumMaterials(); MatNum++)
                            Mesh->SetMaterial(MatNum, OriginalMeshComponent->GetMaterial(MatNum));
                    }
                }
            }
        }
    }
    return true;
}
#pragma optimize("", on)