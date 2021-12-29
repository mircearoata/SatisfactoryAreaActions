// Fill out your copyright notice in the Description page of Project Settings.


#include "AABlueprintFunctionLibrary.h"

#include "Hologram/FGHologram.h"

FTransform UAABlueprintFunctionLibrary::GetHologramSnap(AFGHologram* Hologram, FHitResult HitResult)
{
	EHologramMaterialState PreviousState = Hologram->GetHologramMaterialState();
	const FTransform PreviousHologramTransform = Hologram->GetActorTransform();
	if(!Hologram->TrySnapToActor(HitResult)) {
		Hologram->SetHologramLocationAndRotation(HitResult);
	}
	FTransform NewHologramTransform = Hologram->GetActorTransform();
	Hologram->SetActorTransform(PreviousHologramTransform);
	Hologram->SetPlacementMaterialState(PreviousState);
	Hologram->SetActorHiddenInGame(false);
	return NewHologramTransform;
}

FTransform UAABlueprintFunctionLibrary::GetHologramScroll(AFGHologram* Hologram, const int32 Delta)
{
	EHologramMaterialState PreviousState = Hologram->GetHologramMaterialState();
	const FTransform PreviousHologramTransform = Hologram->GetActorTransform();
	Hologram->Scroll(Delta);
	FTransform NewHologramTransform = Hologram->GetActorTransform();
	Hologram->SetActorTransform(PreviousHologramTransform);
	Hologram->SetPlacementMaterialState(PreviousState);
	Hologram->SetActorHiddenInGame(false);
	return NewHologramTransform;
}

UFGHotbarShortcut* UAABlueprintFunctionLibrary::CreateHotbarShortcut(AFGPlayerState* PlayerState, TSubclassOf<UFGHotbarShortcut> HotbarShortcutClass, int32 Index)
{
	if(!PlayerState || !HotbarShortcutClass) {
		return nullptr;
	}

	return PlayerState->GetOrCreateShortcutOnHotbar(HotbarShortcutClass, PlayerState->mHotbars[PlayerState->GetCurrentHotbarIndex()], Index);
}

void UAABlueprintFunctionLibrary::BroadcastHotbarUpdated(AFGPlayerController* PlayerController, int32 Index)
{
	PlayerController->OnShortcutSet.Broadcast(Index);
	PlayerController->OnShortcutChanged.Broadcast();
}

UAAAreaActionsComponent* UAABlueprintFunctionLibrary::GetAreaActionsComponent(AFGCharacterPlayer* PlayerCharacter)
{
	if(!PlayerCharacter || !PlayerCharacter->GetBuildGun())
		return nullptr;
	return PlayerCharacter->GetBuildGun()->FindComponentByClass<UAAAreaActionsComponent>();
}

bool UAABlueprintFunctionLibrary::InventoriesHaveEnoughItems(const TArray<UFGInventoryComponent*>& Inventories, const TMap<TSubclassOf<UFGItemDescriptor>, int32>& Items, TMap<TSubclassOf<UFGItemDescriptor>, int32>& MissingItems)
{
	MissingItems = Items;
	for(UFGInventoryComponent* Inventory : Inventories)
	{
		TArray<FInventoryStack> Stacks;
		Inventory->GetInventoryStacks(Stacks);
		for(FInventoryStack& Stack : Stacks)
		{
			if(!Stack.HasItems()) continue;
			if(MissingItems.Contains(Stack.Item.ItemClass))
			{
				const int TakenItems = FGenericPlatformMath::Min(Stack.NumItems, MissingItems[Stack.Item.ItemClass]);
				MissingItems[Stack.Item.ItemClass] -= TakenItems;
				if(MissingItems[Stack.Item.ItemClass] == 0)
					MissingItems.Remove(Stack.Item.ItemClass);
			}
		}
	}

	return MissingItems.Num() == 0;
}

bool UAABlueprintFunctionLibrary::TakeItemsFromInventories(const TArray<UFGInventoryComponent*>& Inventories, const TMap<TSubclassOf<UFGItemDescriptor>, int32>& Items, TMap<TSubclassOf<UFGItemDescriptor>, int32>& MissingItems)
{
	if(!InventoriesHaveEnoughItems(Inventories, Items, MissingItems))
		return false;
	
	for(UFGInventoryComponent* Inventory : Inventories)
	{
		TArray<FInventoryStack> Stacks;
		Inventory->GetInventoryStacks(Stacks);
		for(FInventoryStack& Stack : Stacks)
		{
			if(!Stack.HasItems()) continue;
			if(MissingItems.Contains(Stack.Item.ItemClass))
			{
				const int TakenItems = FGenericPlatformMath::Min(Stack.NumItems, MissingItems[Stack.Item.ItemClass]);
				MissingItems[Stack.Item.ItemClass] -= TakenItems;
				if(MissingItems[Stack.Item.ItemClass] == 0)
					MissingItems.Remove(Stack.Item.ItemClass);
                Inventory->Remove(Stack.Item.ItemClass, TakenItems);
			}
		}
	}

	return MissingItems.Num() == 0;
}
