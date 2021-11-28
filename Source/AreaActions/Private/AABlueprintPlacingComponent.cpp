// Fill out your copyright notice in the Description page of Project Settings.

#include "AABlueprintPlacingComponent.h"

#include "AAObjectCollectorArchive.h"
#include "AAObjectReferenceArchive.h"
#include "AAObjectValidatorArchive.h"
#include "AASerializationHelpers.h"
#include "AreaActionsModule.h"
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

UAABlueprintPlacingComponent::UAABlueprintPlacingComponent()
{
}

bool UAABlueprintPlacingComponent::SetBlueprint(UAABlueprint* InBlueprint)
{
    this->Blueprint = InBlueprint;
    return true;
}

FTransform UAABlueprintPlacingComponent::TransformAroundPoint(const FTransform OriginalTransform, const FVector Offset, const FRotator Rotation, const FVector RotationCenter)
{
    const FVector NewLocation = Rotation.RotateVector(OriginalTransform.GetLocation() - RotationCenter) + RotationCenter + Offset;
    const FQuat NewRotation = Rotation.Quaternion() * OriginalTransform.GetRotation();
    return FTransform(NewRotation, NewLocation, OriginalTransform.GetScale3D());
}

void UAABlueprintPlacingComponent::FixReferencesForCopy(const FAABlueprintObjectMap& Copy)
{
    TArray<UObject*> PreviewObjects;
    for(const auto& [_, NewObject] : Copy.GetObjects())
    {
        UAASerializationHelpers::CallPreLoadGame(NewObject);
        PreviewObjects.Add(NewObject);
    }

    TArray<uint8> SerializedData = Blueprint->GetObjectsData().SerializedData;
    FMemoryReader Reader = FMemoryReader(SerializedData, true);
    FAAObjectReferenceArchive Ar2(Reader, PreviewObjects);
    Ar2.SetIsLoading(true);
    Ar2.SetIsSaving(false);
    for(const auto& [_, NewObject] : Copy.GetObjects())
    {
        NewObject->Serialize(Ar2);
    }
    
    for(const auto& [_, NewObject] : Copy.GetObjects())
    {
        UAASerializationHelpers::CallPostLoadGame(NewObject);
    }

    // Remove items
    {
        for(const auto& [_, NewObject] : Copy.GetObjects())
        {
            if(AFGBuildable* Buildable = Cast<AFGBuildable>(NewObject))
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

int UAABlueprintPlacingComponent::AddCopy(const FVector Offset, const FRotator Rotation, const FVector RotationCenter, const bool Relative)
{
    this->CopyLocations.Add(CurrentId, FBlueprintPlaceLocation(Offset, Rotation, RotationCenter, Relative));
    this->Preview.Add(CurrentId);
    int32 ObjectIdx = 0;
    for(const auto& ObjectTOC : this->Blueprint->GetObjectTOC())
    {
        if(ObjectTOC.BuiltWithRecipe)
        {
            AFGBuildableHologram* Hologram = static_cast<AFGBuildableHologram*>(AFGHologram::SpawnHologramFromRecipe(ObjectTOC.BuiltWithRecipe, GetOwner(), FVector::ZeroVector));
            FTransform NewTransform = TransformAroundPoint(ObjectTOC.Transform, Relative ? Blueprint->GetBoundingBox().Rotation.RotateVector(Offset) : Offset, Rotation, RotationCenter);
            Hologram->SetActorTransform(NewTransform);
            Hologram->SetActorHiddenInGame(false);
            Hologram->SetPlacementMaterialState(EHologramMaterialState::HMS_OK);
            this->Preview[CurrentId].Holograms.Add(ObjectIdx, Hologram);
            TArray<UPrimitiveComponent*> Components;
            Hologram->GetComponents<UPrimitiveComponent>(Components);
            for(const UPrimitiveComponent* Component : Components)
            {
                UE_LOG(LogAreaActions, Warning, TEXT("%s, %s, %s"), *Hologram->GetName(), *Component->GetName(), *Component->GetCollisionProfileName().ToString());
            }
        }
        ObjectIdx++;
    }
    return CurrentId++;
}

void UAABlueprintPlacingComponent::MoveCopy(const int CopyId, const FVector Offset, const FRotator Rotation, const FVector RotationCenter, const bool Relative)
{
    FBlueprintPlaceLocation NewLocation = FBlueprintPlaceLocation(Offset, Rotation, RotationCenter, Relative);
    if(this->CopyLocations[CopyId] == NewLocation)
        return;
    
    this->CopyLocations[CopyId] = NewLocation;
    for(const auto& [ObjectIdx, Hologram] : this->Preview[CopyId].Holograms)
    {
        FTransform NewTransform = TransformAroundPoint(Blueprint->GetObjectTOC()[ObjectIdx].Transform, Relative ? Blueprint->GetBoundingBox().Rotation.RotateVector(Offset) : Offset, Rotation, RotationCenter);
        Hologram->SetActorTransform(NewTransform);
    }
}

void UAABlueprintPlacingComponent::RemoveCopy(const int CopyId)
{
    if(!Preview.Contains(CopyId))
        return;
    for(const auto& [_, Hologram] : this->Preview[CopyId].Holograms)
        Hologram->Destroy();
    this->Preview.Remove(CopyId);
    this->CopyLocations.Remove(CopyId);
}

bool UAABlueprintPlacingComponent::CheckItems(TMap<TSubclassOf<UFGItemDescriptor>, int32> RemainingItems, TArray<UFGInventoryComponent*> Inventories, TArray<FInventoryStack>& OutMissingItems, const bool TakeItems) const
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

bool UAABlueprintPlacingComponent::TryTakeItems(TArray<UFGInventoryComponent*> Inventories, TArray<FInventoryStack>& OutMissingItems)
{
    if(this->Preview.Num() == 0)
        return true;
        
    const bool UseBuildCosts = !static_cast<AFGGameState*>(GetWorld()->GetGameState())->GetCheatNoCost();

    TMap<TSubclassOf<UFGItemDescriptor>, int32> ItemsPerCopy;
    if(UseBuildCosts)
        ItemsPerCopy.Append(Blueprint->GetBuildCosts());
    ItemsPerCopy.Append(Blueprint->GetOtherItems());
    
    TMap<TSubclassOf<UFGItemDescriptor>, int32> TotalItems;
    for(auto ItemAmount : ItemsPerCopy)
        TotalItems.Add(ItemAmount.Key, ItemAmount.Value * this->Preview.Num());

    return CheckItems(TotalItems, Inventories, OutMissingItems, true);
}

bool UAABlueprintPlacingComponent::Finish(TArray<UFGInventoryComponent*> Inventories, TArray<FInventoryStack>& OutMissingItems)
{
    if(this->CopyLocations.Num() == 0)
        return true;

    if(!TryTakeItems(Inventories, OutMissingItems))
        return false;

    for(const auto& [CopyId, CopyLocation] : this->CopyLocations)
    {
        FAABlueprintObjectMap CurrentCopy;
        int32 ObjectIdx = 0;
        for(const auto& BuildingTOC : this->Blueprint->GetObjectTOC())
        {
            UObject* NewObj;
            if(BuildingTOC.Class->IsChildOf(AActor::StaticClass()))
            {
                FTransform NewTransform = TransformAroundPoint(BuildingTOC.Transform, CopyLocation.Relative ? Blueprint->GetBoundingBox().Rotation.RotateVector(CopyLocation.Offset) : CopyLocation.Offset, CopyLocation.Rotation, CopyLocation.RotationCenter);
                FActorSpawnParameters Params;
                Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                Params.bDeferConstruction = true;
                // Params.Owner = BuildingTOC.Owner; // Must obtain after all objects are constructed
                AActor* NewActor;
                if(BuildingTOC.Class->IsChildOf(AFGBuildable::StaticClass()))
                {
                    const TSubclassOf<AFGBuildable> BuildableClass(BuildingTOC.Class);
                    NewActor = AFGBuildableSubsystem::Get(GetWorld())->BeginSpawnBuildable(BuildableClass, NewTransform);
                }
                else
                    NewActor = this->GetWorld()->SpawnActor<AActor>(BuildingTOC.Class, NewTransform, Params);
                NewActor->bDeferBeginPlay = true;
                NewActor->FinishSpawning(FTransform::Identity, true);
                NewObj = NewActor;
            }
            else
            {
                UObject* Outer = CurrentCopy.GetObjectReference<UObject>(BuildingTOC.Outer);
                if(BuildingTOC.Outer.Idx != -1)
                    NewObj = FindObject<UObject>(Outer, *BuildingTOC.Name.ToString());
                else
                    NewObj = NewObject<UObject>(Outer, BuildingTOC.Class, NAME_None, BuildingTOC.Flags);
                if(!NewObj)
                    NewObj = NewObject<UObject>(Outer, BuildingTOC.Class, BuildingTOC.Name, BuildingTOC.Flags);
            }
            CurrentCopy.AddObject(ObjectIdx, NewObj);
            ObjectIdx++;
        }
    
        this->FixReferencesForCopy(CurrentCopy);
    
        for(const auto& [_, NewObject] : CurrentCopy.GetObjects())
        {
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

AFGBuildableHologram* UAABlueprintPlacingComponent::GetPreviewHologram(int CopyId, int32 Buildable)
{
    if(!this->Preview.Contains(CopyId)) return nullptr;
    if(!this->Preview[CopyId].Holograms.Contains(Buildable)) return nullptr;
    return this->Preview[CopyId].Holograms[Buildable];
}

void UAABlueprintPlacingComponent::GetAllPreviewHolograms(TArray<AFGBuildableHologram*>& OutPreviewHolograms)
{
    for(const auto& [CopyId, CopyPreview] : this->Preview)
    {
        for(const auto& [_, Hologram] : CopyPreview.Holograms)
        {
            OutPreviewHolograms.Add(Hologram);
        }
    }
}

int32 UAABlueprintPlacingComponent::GetHologramObjectIdx(AFGBuildableHologram* Hologram)
{
    for(const auto& [CopyId, CopyPreview] : this->Preview)
    {
        const int32* KeyPtr = CopyPreview.Holograms.FindKey(Hologram);
        if(KeyPtr)
            return *KeyPtr;
    }
    return -1;
}
