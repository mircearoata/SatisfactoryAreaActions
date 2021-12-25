#include "Actions/AALoadBlueprint.h"

#include "AABlueprint.h"
#include "AABlueprintFunctionLibrary.h"
#include "AABlueprintSystem.h"
#include "AAAreaActionsComponent.h"
#include "FGCharacterPlayer.h"
#include "FGInventoryComponent.h"
#include "FGPlayerController.h"
#include "Buildables/FGBuildableStorage.h"
#include "Hologram/FGBuildableHologram.h"

AAALoadBlueprint::AAALoadBlueprint() {
	this->BlueprintPlacingComponent = CreateDefaultSubobject<UAABlueprintPlacingComponent>(TEXT("BlueprintPlacing"));
	this->DeltaPosition = FVector::ZeroVector;
	this->DeltaRotation = FRotator::ZeroRotator;

	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AAALoadBlueprint::Tick(float DeltaSeconds)
{
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
				if(AFGBuildableHologram* AnchorHologram = BlueprintPlacingComponent->GetPreviewHologram(0, AnchorIdx))
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
}

void AAALoadBlueprint::EnableInput(APlayerController* PlayerController)
{
	Super::EnableInput(PlayerController);
	
	InputComponent->BindAction("PrimaryFire", EInputEvent::IE_Pressed, this, &AAALoadBlueprint::PrimaryFire);
	InputComponent->BindAction("SecondaryFire", EInputEvent::IE_Pressed, this, &AAALoadBlueprint::PrimaryFire);
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
	}
	else if(bIsPlacing)
	{
		bIsPlacing = false;
		ScrollUpInputActionBinding->bConsumeInput = false;
		ScrollDownInputActionBinding->bConsumeInput = false;
	}
	AreaActionsComponent->ToggleBuildMenu();
}

void AAALoadBlueprint::ScrollUp()
{
	if(AnchorIdx != INDEX_NONE)
	{
		if(AFGBuildableHologram* Hologram = BlueprintPlacingComponent->GetPreviewHologram(0, AnchorIdx))
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
		if(AFGBuildableHologram* Hologram = BlueprintPlacingComponent->GetPreviewHologram(0, AnchorIdx))
		{
			FTransform Scrolled = UAABlueprintFunctionLibrary::GetHologramScroll(Hologram, -1);
			SetDeltaFromAnchorTransform(Scrolled);
			return;
		}
	}
	DeltaRotation = DeltaRotation.Add(0, -10, 0);
}

void AAALoadBlueprint::SetDeltaFromAnchorTransform(FTransform HologramTransform)
{
	DeltaPosition = BlueprintPlacingComponent->GetBounds().Rotation.UnrotateVector(HologramTransform.GetTranslation() - Blueprint->GetObjectTOC()[AnchorIdx].Transform.GetLocation());
	DeltaRotation = HologramTransform.GetRotation().Rotator() - Blueprint->GetObjectTOC()[AnchorIdx].Transform.GetRotation().Rotator();

	DeltaRotation = FRotator(FGenericPlatformMath::RoundToInt(DeltaRotation.Pitch), FGenericPlatformMath::RoundToInt(DeltaRotation.Yaw), FGenericPlatformMath::RoundToInt(DeltaRotation.Roll));
}

void AAALoadBlueprint::PathSelected(const FString BlueprintPath)
{
	Blueprint = GetGameInstance()->GetSubsystem<UAABlueprintSystem>()->LoadBlueprint(BlueprintPath);
	if (!Blueprint) {
		// Error handling
		this->Done();
	}
	else {
		this->BlueprintPlacingComponent->SetBlueprint(Blueprint);
		this->BlueprintPlacingComponent->AddCopy(FVector::ZeroVector, FRotator::ZeroRotator);
		this->ShowPlaceBlueprintWidget();
	}
}

void AAALoadBlueprint::OnCancel_Implementation()
{
	this->BlueprintPlacingComponent->RemoveCopy(0);
}

void AAALoadBlueprint::Preview()
{
	this->BlueprintPlacingComponent->MoveCopy(0, DeltaPosition, DeltaRotation, AnchorIdx != INDEX_NONE ? Blueprint->GetObjectTOC()[AnchorIdx].Transform.GetLocation() : BlueprintPlacingComponent->GetBuildingsCenter());
}

bool AAALoadBlueprint::Finish(const TArray<UFGInventoryComponent*>& Inventories, TArray<FInventoryStack>& MissingItems)
{
	// TODO Make this whole action be like Copy. Also probably shouldn't be an action
	this->Preview();
	TMap<TSubclassOf<UFGItemDescriptor>, int32> MissingItemsMap;
	return this->BlueprintPlacingComponent->Finish(Inventories, MissingItemsMap);
}

void AAALoadBlueprint::RemoveMissingItemsWidget()
{
	MissingItemsWidget = nullptr;
}

void AAALoadBlueprint::EnterPickAnchor()
{
	bIsPickingAnchor = true;
	AreaActionsComponent->ToggleBuildMenu();
}

void AAALoadBlueprint::EnterPlacing()
{
	ScrollUpInputActionBinding->bConsumeInput = true;
	ScrollDownInputActionBinding->bConsumeInput = true;
	bIsPlacing = true;
	AreaActionsComponent->ToggleBuildMenu();
}