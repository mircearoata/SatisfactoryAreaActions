// Fill out your copyright notice in the Description page of Project Settings.


#include "AABlueprintFunctionLibrary.h"

#include "Hologram/FGHologram.h"

FTransform UAABlueprintFunctionLibrary::GetHologramSnap(AFGHologram* Hologram, FHitResult HitResult)
{
	const FTransform PreviousHologramTransform = Hologram->GetActorTransform();
	if(!Hologram->TrySnapToActor(HitResult)) {
		Hologram->SetHologramLocationAndRotation(HitResult);
	}
	FTransform NewHologramTransform = Hologram->GetActorTransform();
	Hologram->SetActorTransform(PreviousHologramTransform);
	Hologram->SetPlacementMaterialState(EHologramMaterialState::HMS_OK);
	Hologram->SetActorHiddenInGame(false);
	return NewHologramTransform;
}

FTransform UAABlueprintFunctionLibrary::GetHologramScroll(AFGHologram* Hologram, const int32 Delta)
{
	const FTransform PreviousHologramTransform = Hologram->GetActorTransform();
	Hologram->Scroll(Delta);
	FTransform NewHologramTransform = Hologram->GetActorTransform();
	Hologram->SetActorTransform(PreviousHologramTransform);
	Hologram->SetPlacementMaterialState(EHologramMaterialState::HMS_OK);
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
