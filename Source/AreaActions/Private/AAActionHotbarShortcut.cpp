// Copyright Coffee Stain Studios. All Rights Reserved.


#include "AAActionHotbarShortcut.h"

#include "AABlueprintFunctionLibrary.h"
#include "AAAreaActionsComponent.h"
#include "FGPlayerController.h"

void UAAActionHotbarShortcut::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAAActionHotbarShortcut, mActionToActivate);
}

void UAAActionHotbarShortcut::SetAction(TSubclassOf<AAAAction> Action)
{
	mActionToActivate = Action;
}

void UAAActionHotbarShortcut::Execute_Implementation(AFGPlayerController* Owner)
{
	FText Error;
	UAAAreaActionsComponent* AreaActionsComponent = UAABlueprintFunctionLibrary::GetAreaActionsComponent(static_cast<AFGCharacterPlayer*>(Owner->GetPawn()));
	AreaActionsComponent->RunAction(mActionToActivate, Error);
}

bool UAAActionHotbarShortcut::IsValidShortcut_Implementation(AFGPlayerController* Owner) const
{
	return IsValid(mActionToActivate);
}

UTexture2D* UAAActionHotbarShortcut::GetDisplayImage_Implementation() const
{
	return AAAAction::GetActionIcon(mActionToActivate);
}

bool UAAActionHotbarShortcut::IsActive_Implementation(AFGPlayerController* Owner) const
{
	UAAAreaActionsComponent* AreaActionsComponent = UAABlueprintFunctionLibrary::GetAreaActionsComponent(static_cast<AFGCharacterPlayer*>(Owner->GetPawn()));
	if(!AreaActionsComponent)
		return false;
	const AAAAction* CurrentAction = AreaActionsComponent->CurrentAction;
	return IsValid(CurrentAction) && CurrentAction->GetClass() == mActionToActivate;
}

void UAAActionHotbarShortcut::OnRep_Action()
{
}
