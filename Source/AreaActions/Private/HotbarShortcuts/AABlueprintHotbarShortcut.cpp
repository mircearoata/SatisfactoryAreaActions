// Copyright Coffee Stain Studios. All Rights Reserved.


#include "HotbarShortcuts/AABlueprintHotbarShortcut.h"

#include "AABlueprintFunctionLibrary.h"
#include "AAAreaActionsComponent.h"
#include "AABlueprintSystem.h"
#include "FGPlayerController.h"
#include "Actions/AALoadBlueprint.h"

void UAABlueprintHotbarShortcut::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAABlueprintHotbarShortcut, mBlueprintFileNameToActivate);
	DOREPLIFETIME(UAABlueprintHotbarShortcut, mBlueprintHeaderToActivate);
}

void UAABlueprintHotbarShortcut::SetBlueprintHeader(FAABlueprintHeader BlueprintHeader)
{
	mBlueprintHeaderToActivate = BlueprintHeader;
}

void UAABlueprintHotbarShortcut::SetBlueprintFileName(FString BlueprintFileName)
{
	mBlueprintFileNameToActivate = BlueprintFileName;
}

bool UAABlueprintHotbarShortcut::IsValidShortcut_Implementation(AFGPlayerController* Owner) const
{
	return mBlueprintFileNameToActivate.Len() > 0;
}

UTexture2D* UAABlueprintHotbarShortcut::GetDisplayImage_Implementation() const
{
	return UAABlueprintSystem::GetBlueprintIcon(mBlueprintHeaderToActivate);
}

bool UAABlueprintHotbarShortcut::IsActive_Implementation(AFGPlayerController* Owner) const
{
	UAAAreaActionsComponent* AreaActionsComponent = UAABlueprintFunctionLibrary::GetAreaActionsComponent(static_cast<AFGCharacterPlayer*>(Owner->GetPawn()));
	if(!AreaActionsComponent)
		return false;
	const AAAAction* CurrentAction = AreaActionsComponent->CurrentAction;
	if(const AAALoadBlueprint* LoadBlueprintAction = Cast<AAALoadBlueprint>(CurrentAction))
		return LoadBlueprintAction->GetBlueprintFileName() == mBlueprintFileNameToActivate;
	return false;
}

void UAABlueprintHotbarShortcut::OnRep_BlueprintHeader()
{
}


void UAABlueprintHotbarShortcut::OnRep_BlueprintFileName()
{
}
