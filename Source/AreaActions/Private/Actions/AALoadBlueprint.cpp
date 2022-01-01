#include "Actions/AALoadBlueprint.h"

#include "AABlueprint.h"
#include "AABlueprintFunctionLibrary.h"
#include "AABlueprintSystem.h"
#include "AAAreaActionsComponent.h"
#include "FGCharacterPlayer.h"
#include "FGPlayerController.h"
#include "Hologram/FGBuildableHologram.h"
#include "FGGameState.h"

AAALoadBlueprint::AAALoadBlueprint() {
	this->BlueprintPlacingComponent = CreateDefaultSubobject<UAABlueprintPlacingComponent>(TEXT("BlueprintPlacing"));
	this->DeltaPosition = FVector::ZeroVector;
	this->DeltaRotation = FRotator::ZeroRotator;

	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AAALoadBlueprint::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(bIsPlacing)
	{
		TArray<AFGBuildableHologram*> Holograms;
		BlueprintPlacingComponent->GetAllPreviewHolograms(Holograms);
		TArray<AActor*> HologramsActors(MoveTemp(Holograms));
		FHitResult HitResult;
		if(AreaActionsComponent->RaycastMouseWithRange(HitResult, true, true, true, HologramsActors))
		{
			if(AnchorIdx != INDEX_NONE)
			{
				if(AFGBuildableHologram* AnchorHologram = BlueprintPlacingComponent->GetPreviewHologram(CurrentCopy, AnchorIdx))
				{
					FTransform Snapped = UAABlueprintFunctionLibrary::GetHologramSnap(AnchorHologram, HitResult);
					SetDeltaFromAnchorTransform(Snapped);
				}
			}
			else
			{
				DeltaPosition = BlueprintPlacingComponent->GetBounds().Rotation.UnrotateVector(HitResult.Location - BlueprintPlacingComponent->GetBounds().Center);
			}
		}
		Preview();
	}
	BlueprintPlacingComponent->SetHologramState(CanFinish() ? EHologramMaterialState::HMS_OK : EHologramMaterialState::HMS_ERROR);
}

void AAALoadBlueprint::EnableInput(APlayerController* PlayerController)
{
	Super::EnableInput(PlayerController);
	
	InputComponent->BindAction("PrimaryFire", EInputEvent::IE_Pressed, this, &AAALoadBlueprint::PrimaryFire);
	InputComponent->BindAction("SecondaryFire", EInputEvent::IE_Pressed, this, &AAALoadBlueprint::OpenMenu);
	InputComponent->BindAction("AreaActions.PickAnchor", EInputEvent::IE_Pressed, this, &AAALoadBlueprint::EnterPickAnchor);
	ScrollUpInputActionBinding = &InputComponent->BindAction("BuildGunScrollUp_PhotoModeFOVUp", EInputEvent::IE_Pressed, this, &AAALoadBlueprint::ScrollUp);
	ScrollDownInputActionBinding = &InputComponent->BindAction("BuildGunScrollDown_PhotoModeFOVDown", EInputEvent::IE_Pressed, this, &AAALoadBlueprint::ScrollDown);
	ScrollUpInputActionBinding->bConsumeInput = false;
	ScrollDownInputActionBinding->bConsumeInput = false;
}

void AAALoadBlueprint::PrimaryFire()
{
	if(bIsPickingAnchor)
	{
		bIsPickingAnchor = false;
		FHitResult HitResult;
		TArray<AFGBuildableHologram*> PreviewHolograms;
		BlueprintPlacingComponent->GetAllPreviewHolograms(PreviewHolograms);
		if(AreaActionsComponent->RaycastMouseWithRange(HitResult, true, true, true) && PreviewHolograms.Contains(HitResult.Actor))
		{
			AnchorIdx = BlueprintPlacingComponent->GetHologramObjectIdx(static_cast<AFGBuildableHologram*>(HitResult.Actor.Get()));
		}
		else
		{
			AnchorIdx = INDEX_NONE;
		}
		EnterPlacing();
	}
	else if(bIsPlacing)
	{
		if(Finish())
		{
			CreateNewCopy();
		}
	}
}

void AAALoadBlueprint::OpenMenu()
{
	bIsPickingAnchor = false;
	bIsPlacing = false;
	ScrollUpInputActionBinding->bConsumeInput = false;
	ScrollDownInputActionBinding->bConsumeInput = false;
	AreaActionsComponent->ShowBuildMenu();
}

void AAALoadBlueprint::ScrollUp()
{
	if(AnchorIdx != INDEX_NONE)
	{
		if(AFGBuildableHologram* Hologram = BlueprintPlacingComponent->GetPreviewHologram(CurrentCopy, AnchorIdx))
		{
			FTransform Scrolled = UAABlueprintFunctionLibrary::GetHologramScroll(Hologram, 1);
			SetDeltaFromAnchorTransform(Scrolled);
			return;
		}
	}
	DeltaRotation = DeltaRotation.Add(0, 10, 0);
}

void AAALoadBlueprint::ScrollDown()
{
	if(AnchorIdx != INDEX_NONE)
	{
		if(AFGBuildableHologram* Hologram = BlueprintPlacingComponent->GetPreviewHologram(CurrentCopy, AnchorIdx))
		{
			FTransform Scrolled = UAABlueprintFunctionLibrary::GetHologramScroll(Hologram, -1);
			SetDeltaFromAnchorTransform(Scrolled);
			return;
		}
	}
	DeltaRotation = DeltaRotation.Add(0, -10, 0);
}

void AAALoadBlueprint::CreateNewCopy()
{
	CurrentCopy = this->BlueprintPlacingComponent->AddCopy(FVector::ZeroVector, FRotator::ZeroRotator, BlueprintPlacingComponent->GetBuildingsCenter(), true);
}

void AAALoadBlueprint::SetDeltaFromAnchorTransform(FTransform HologramTransform)
{
	DeltaPosition = BlueprintPlacingComponent->GetBounds().Rotation.UnrotateVector(HologramTransform.GetTranslation() - Blueprint->GetObjectTOC()[AnchorIdx].Transform.GetLocation());
	DeltaRotation = HologramTransform.GetRotation().Rotator() - Blueprint->GetObjectTOC()[AnchorIdx].Transform.GetRotation().Rotator();

	DeltaRotation = FRotator(FGenericPlatformMath::RoundToInt(DeltaRotation.Pitch), FGenericPlatformMath::RoundToInt(DeltaRotation.Yaw), FGenericPlatformMath::RoundToInt(DeltaRotation.Roll));
}

bool AAALoadBlueprint::SetPath(const FString InBlueprintName)
{
	this->BlueprintFileName = InBlueprintName;
	Blueprint = GetGameInstance()->GetSubsystem<UAABlueprintSystem>()->LoadBlueprint(InBlueprintName);
	if (!Blueprint) {
		return false;
	}
	this->BlueprintPlacingComponent->SetBlueprint(Blueprint);
	return true;
}

void AAALoadBlueprint::OnCancel_Implementation()
{
	this->BlueprintPlacingComponent->RemoveCopy(CurrentCopy);
}

void AAALoadBlueprint::Preview()
{
	this->BlueprintPlacingComponent->MoveCopy(CurrentCopy, DeltaPosition, DeltaRotation, AnchorIdx != INDEX_NONE ? Blueprint->GetObjectTOC()[AnchorIdx].Transform.GetLocation() : BlueprintPlacingComponent->GetBuildingsCenter());
}

bool AAALoadBlueprint::Finish()
{
	if(!CanFinish())
		return false;
	this->Preview();
	TMap<TSubclassOf<UFGItemDescriptor>, int32> MissingItemsMap;
	const bool ReturnValue = this->BlueprintPlacingComponent->Finish(GetInventories(), MissingItemsMap);
	return ReturnValue; 
}

bool AAALoadBlueprint::CanFinish() const
{
	if(!Blueprint)
		return false;
    const bool UseBuildCosts = !static_cast<AFGGameState*>(GetWorld()->GetGameState())->GetCheatNoCost();
	if(!UseBuildCosts)
		return true;
	TMap<TSubclassOf<UFGItemDescriptor>, int32> MissingItemsMap;
	return UAABlueprintFunctionLibrary::InventoriesHaveEnoughItems(GetInventories(), BlueprintPlacingComponent->GetRequiredItems(), MissingItemsMap);
}

void AAALoadBlueprint::RemoveMissingItemsWidget()
{
	MissingItemsWidget = nullptr;
}

void AAALoadBlueprint::EnterPickAnchor()
{
	ScrollUpInputActionBinding->bConsumeInput = false;
	ScrollDownInputActionBinding->bConsumeInput = false;
	bIsPlacing = false;
	bIsPickingAnchor = true;
	AreaActionsComponent->HideBuildMenu();
}

void AAALoadBlueprint::EnterPlacing()
{
	ScrollUpInputActionBinding->bConsumeInput = true;
	ScrollDownInputActionBinding->bConsumeInput = true;
	bIsPlacing = true;
	bIsPickingAnchor = false;
	AreaActionsComponent->HideBuildMenu();
}