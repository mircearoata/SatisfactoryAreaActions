// Fill out your copyright notice in the Description page of Project Settings.

#include "Actions/AACopy.h"

#include "AABlueprintFunctionLibrary.h"
#include "AAAreaActionsComponent.h"
#include "FGOutlineComponent.h"
#include "FGPlayerController.h"
#include "Buildables/FGBuildableStorage.h"

AAACopy::AAACopy(): Super() {	
	this->CopyBuildingsComponent = CreateDefaultSubobject<UAACopyBuildingsComponent>(TEXT("CopyBuildings"));
	this->DeltaPosition = FVector::ZeroVector;
	this->DeltaRotation = FRotator::ZeroRotator;

	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AAACopy::Tick(float DeltaSeconds)
{
	if(bIsPlacing)
	{
		TArray<AFGBuildableHologram*> Holograms;
		CopyBuildingsComponent->GetAllPreviewHolograms(Holograms);
		TArray<AActor*> HologramsActors(MoveTemp(Holograms));
		FHitResult HitResult;
		if(AreaActionsComponent->RaycastMouseWithRange(HitResult, true, true, true, HologramsActors))
		{
			if(Anchor)
			{
				if(AFGBuildableHologram* AnchorHologram = CopyBuildingsComponent->GetPreviewHologram(0, Anchor))
				{
					FTransform Snapped = UAABlueprintFunctionLibrary::GetHologramSnap(AnchorHologram, HitResult);
					SetDeltaFromAnchorTransform(Snapped);
				}
			}
			else
			{
				DeltaPosition = CopyBuildingsComponent->GetBounds().Rotation.UnrotateVector(HitResult.Location - CopyBuildingsComponent->GetBounds().Center);
			}
		}
		Preview();
	}
	CopyBuildingsComponent->SetHologramState(CanFinish() ? EHologramMaterialState::HMS_OK : EHologramMaterialState::HMS_ERROR);
}

void AAACopy::EnableInput(APlayerController* PlayerController)
{
	Super::EnableInput(PlayerController);

	InputComponent->BindAction("PrimaryFire", EInputEvent::IE_Pressed, this, &AAACopy::PrimaryFire);
	InputComponent->BindAction("SecondaryFire", EInputEvent::IE_Pressed, this, &AAACopy::OpenMenu);
	InputComponent->BindAction("AreaActions.PickAnchor", EInputEvent::IE_Pressed, this, &AAACopy::EnterPickAnchor);
	ScrollUpInputActionBinding = &InputComponent->BindAction("BuildGunScrollUp_PhotoModeFOVUp", EInputEvent::IE_Pressed, this, &AAACopy::ScrollUp);
	ScrollDownInputActionBinding = &InputComponent->BindAction("BuildGunScrollDown_PhotoModeFOVDown", EInputEvent::IE_Pressed, this, &AAACopy::ScrollDown);
	ScrollUpInputActionBinding->bConsumeInput = false;
	ScrollDownInputActionBinding->bConsumeInput = false;
}

void AAACopy::PrimaryFire()
{
	if(bIsPickingAnchor)
	{
		bIsPickingAnchor = false;
		FHitResult HitResult;
		TMap<AFGBuildable*, AFGBuildableHologram*> PreviewHolograms;
		CopyBuildingsComponent->GetBuildingHolograms(0, PreviewHolograms);
		if(AreaActionsComponent->RaycastMouseWithRange(HitResult, true, true, true))
		{
			if(Actors.Contains(HitResult.Actor))
			{
				Anchor = static_cast<AFGBuildable*>(HitResult.Actor.Get());
			}
			if(auto* Buildable = PreviewHolograms.FindKey(static_cast<AFGBuildableHologram*>(HitResult.Actor.Get())))
			{
				Anchor = static_cast<AFGBuildable*>(*Buildable);
			}
		}
		else
		{
			Anchor = nullptr;
		}
		EnterPlacing();
	}
	else if(bIsPlacing)
	{
		if(Finish())
		{
			bIsPlacing = false;
			ScrollUpInputActionBinding->bConsumeInput = false;
			ScrollDownInputActionBinding->bConsumeInput = false;
			Done();
		}
	}
}

void AAACopy::OpenMenu()
{
	bIsPickingAnchor = false;
	bIsPlacing = false;
	ScrollUpInputActionBinding->bConsumeInput = false;
	ScrollDownInputActionBinding->bConsumeInput = false;
	AreaActionsComponent->ShowBuildMenu();
}

void AAACopy::ScrollUp()
{
	if(Anchor)
	{
		if(AFGBuildableHologram* Hologram = CopyBuildingsComponent->GetPreviewHologram(0, Anchor))
		{
			FTransform Scrolled = UAABlueprintFunctionLibrary::GetHologramScroll(Hologram, 1);
			SetDeltaFromAnchorTransform(Scrolled);
			return;
		}
	}
	DeltaRotation = DeltaRotation.Add(0, 10, 0);
}

void AAACopy::ScrollDown()
{
	if(Anchor)
	{
		if(AFGBuildableHologram* Hologram = CopyBuildingsComponent->GetPreviewHologram(0, Anchor))
		{
			FTransform Scrolled = UAABlueprintFunctionLibrary::GetHologramScroll(Hologram, -1);
			SetDeltaFromAnchorTransform(Scrolled);
			return;
		}
	}
	DeltaRotation = DeltaRotation.Add(0, -10, 0);
}

void AAACopy::SetDeltaFromAnchorTransform(FTransform HologramTransform)
{
	DeltaPosition = CopyBuildingsComponent->GetBounds().Rotation.UnrotateVector(HologramTransform.GetTranslation() - Anchor->GetActorLocation());
	DeltaRotation = HologramTransform.GetRotation().Rotator() - Anchor->GetActorRotation();

	DeltaRotation = FRotator(FGenericPlatformMath::RoundToInt(DeltaRotation.Pitch), FGenericPlatformMath::RoundToInt(DeltaRotation.Yaw), FGenericPlatformMath::RoundToInt(DeltaRotation.Roll));
}

void AAACopy::OnCancel_Implementation()
{
	this->CopyBuildingsComponent->RemoveCopy(0);
}

void AAACopy::Preview()
{
	this->CopyBuildingsComponent->MoveCopy(0, DeltaPosition, DeltaRotation, Anchor ? Anchor->GetActorLocation() : CopyBuildingsComponent->GetBuildingsCenter());
}

bool AAACopy::Finish()
{
	if(!CanFinish())
		return false;
	this->Preview();
	TMap<TSubclassOf<UFGItemDescriptor>, int32> MissingItemsMap;
	const bool ReturnValue = this->CopyBuildingsComponent->Finish(GetInventories(), MissingItemsMap);
	return ReturnValue; 
}

bool AAACopy::CanFinish() const
{
	TMap<TSubclassOf<UFGItemDescriptor>, int32> MissingItemsMap;
	return UAABlueprintFunctionLibrary::InventoriesHaveEnoughItems(GetInventories(), CopyBuildingsComponent->GetRequiredItems(), MissingItemsMap);
}

void AAACopy::RemoveMissingItemsWidget()
{
	MissingItemsWidget = nullptr;
}

void AAACopy::EnterPickAnchor()
{
	ScrollUpInputActionBinding->bConsumeInput = false;
	ScrollDownInputActionBinding->bConsumeInput = false;
	bIsPlacing = false;
	bIsPickingAnchor = true;
	AreaActionsComponent->HideBuildMenu();
}

void AAACopy::EnterPlacing()
{
	ScrollUpInputActionBinding->bConsumeInput = true;
	ScrollDownInputActionBinding->bConsumeInput = true;
	bIsPlacing = true;
	bIsPickingAnchor = false;
	AreaActionsComponent->HideBuildMenu();
}
