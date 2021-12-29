// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AAAreaActionsComponent.h"
#include "FGHotbarShortcut.h"
#include "FGPlayerState.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AABlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class AREAACTIONS_API UAABlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	public:
	UFUNCTION(BlueprintCallable, Category="AreaActions|Hologram")
	static FTransform GetHologramSnap(class AFGHologram* Hologram, FHitResult HitResult);
	
	UFUNCTION(BlueprintCallable, Category="AreaActions|Hologram")
	static FTransform GetHologramScroll(class AFGHologram* Hologram, int32 Delta);

	UFUNCTION(BlueprintCallable, Category="AreaActions|Hotbar", meta=(DeterminesOutputType="HotbarShortcutClass"))
	static UFGHotbarShortcut* CreateHotbarShortcut(AFGPlayerState* PlayerState, TSubclassOf<UFGHotbarShortcut> HotbarShortcutClass, int32 Index);

	UFUNCTION(BlueprintCallable, Category="AreaActions|Hotbar")
	static void BroadcastHotbarUpdated(AFGPlayerController* PlayerController, int32 Index);

	UFUNCTION(BlueprintPure, Category="AreaActions")
	static UAAAreaActionsComponent* GetAreaActionsComponent(AFGCharacterPlayer* PlayerCharacter);

	UFUNCTION(BlueprintPure, Category="AreaActions")
	static bool InventoriesHaveEnoughItems(const TArray<UFGInventoryComponent*>& Inventories, const TMap<TSubclassOf<UFGItemDescriptor>, int32>& Items, TMap<TSubclassOf<UFGItemDescriptor>, int32>& MissingItems);

	UFUNCTION(BlueprintPure, Category="AreaActions")
	static bool TakeItemsFromInventories(const TArray<UFGInventoryComponent*>& Inventories, const TMap<TSubclassOf<UFGItemDescriptor>, int32>& Items, TMap<TSubclassOf<UFGItemDescriptor>, int32>& MissingItems);
};
