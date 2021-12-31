// Copyright Coffee Stain Studios. All Rights Reserved.


#include "HotbarShortcuts/AASelectionModeHotbarShortcut.h"

#include "AABlueprintFunctionLibrary.h"
#include "AAAreaActionsComponent.h"
#include "FGPlayerController.h"

void UAASelectionModeHotbarShortcut::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAASelectionModeHotbarShortcut, mSelectionModeToActivate);
}

void UAASelectionModeHotbarShortcut::SetSelectionMode(EAASelectionMode SelectionMode)
{
	mSelectionModeToActivate = SelectionMode;
}

void UAASelectionModeHotbarShortcut::Execute_Implementation(AFGPlayerController* Owner)
{
	FText Error;
	UAAAreaActionsComponent* AreaActionsComponent = UAABlueprintFunctionLibrary::GetAreaActionsComponent(static_cast<AFGCharacterPlayer*>(Owner->GetPawn()));
	AreaActionsComponent->EnterSelectionMode(mSelectionModeToActivate, SelectionActorClass);
}

bool UAASelectionModeHotbarShortcut::IsValidShortcut_Implementation(AFGPlayerController* Owner) const
{
	return true;
}

bool UAASelectionModeHotbarShortcut::IsActive_Implementation(AFGPlayerController* Owner) const
{
	UAAAreaActionsComponent* AreaActionsComponent = UAABlueprintFunctionLibrary::GetAreaActionsComponent(static_cast<AFGCharacterPlayer*>(Owner->GetPawn()));
	if(!AreaActionsComponent)
		return false;
	return AreaActionsComponent->SelectionActor && AreaActionsComponent->SelectionActor->SelectionMode == mSelectionModeToActivate;
}

void UAASelectionModeHotbarShortcut::OnRep_SelectionMode()
{
}
