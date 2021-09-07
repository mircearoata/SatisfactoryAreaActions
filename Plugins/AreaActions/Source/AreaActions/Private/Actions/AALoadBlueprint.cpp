#include "Actions/AALoadBlueprint.h"

#include "AABlueprint.h"
#include "AABlueprintFunctionLibrary.h"
#include "AAEquipment.h"
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
		if(AAEquipment->RaycastMouseWithRange(HitResult, true, true, true, HologramsActors))
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

void AAALoadBlueprint::PrimaryFire()
{
	if(bIsPickingAnchor)
	{
		bIsPickingAnchor = false;
		FHitResult HitResult;
		TArray<AFGBuildableHologram*> PreviewHolograms;
		BlueprintPlacingComponent->GetAllPreviewHolograms(PreviewHolograms);
		if(AAEquipment->RaycastMouseWithRange(HitResult, true, true, true) && PreviewHolograms.Contains(HitResult.Actor))
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
	AAEquipment->OpenMainWidget();
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

void AAALoadBlueprint::EquipmentEquipped(AAAEquipment* Equipment)
{
	Super::EquipmentEquipped(Equipment);
	if(!InputComponent->HasBindings())
	{
		InputComponent->BindAction("PrimaryFire", EInputEvent::IE_Pressed, this, &AAALoadBlueprint::PrimaryFire);
		InputComponent->BindAction("SecondaryFire", EInputEvent::IE_Pressed, this, &AAALoadBlueprint::PrimaryFire);
		ScrollUpInputActionBinding = &InputComponent->BindAction("BuildGunScrollUp_PhotoModeFOVUp", EInputEvent::IE_Pressed, this, &AAALoadBlueprint::ScrollUp);
		ScrollDownInputActionBinding = &InputComponent->BindAction("BuildGunScrollDown_PhotoModeFOVDown", EInputEvent::IE_Pressed, this, &AAALoadBlueprint::ScrollDown);
		ScrollUpInputActionBinding->bConsumeInput = false;
		ScrollDownInputActionBinding->bConsumeInput = false;
	}
}

void AAALoadBlueprint::Run_Implementation() {
	this->ShowSelectBlueprintWidget();
}

void AAALoadBlueprint::NameSelected(const FString BlueprintName)
{
	Blueprint = NewObject<UAABlueprint>(GetWorld());
	if (!Blueprint->LoadBlueprint(UAABlueprint::GetBlueprintPath(BlueprintName))) {
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

void AAALoadBlueprint::Finish()
{
	this->Preview();
	TArray<UFGInventoryComponent*> Inventories;
	TArray<FInventoryStack> MissingItems;
	AFGCharacterPlayer* Player = static_cast<AFGCharacterPlayer*>(this->AAEquipment->GetOwningController()->GetPawn());

	for(TActorIterator<AFGBuildableStorage> StorageIt(GetWorld()); StorageIt; ++StorageIt)
	{
		if(FVector::Distance(StorageIt->GetActorLocation(), Player->GetActorLocation()) < 10000)
		{
			Inventories.Add(StorageIt->GetInitialStorageInventory());
		}
	}
	
	Inventories.Add(Player->GetInventory());
	if(!this->BlueprintPlacingComponent->Finish(Inventories, MissingItems))
	{
		FString MissingItemsString = TEXT("");
		for(const auto Stack : MissingItems)
		{
			MissingItemsString += FString::Printf(
                TEXT("%s%d %s"), MissingItemsString.Len() != 0 ? TEXT(", ") : TEXT(""),
                Stack.NumItems,
                *UFGItemDescriptor::GetItemName(Stack.Item.ItemClass).ToString());
		}
		FOnMessageOk MessageOk;
		MessageOk.BindDynamic(this, &AAALoadBlueprint::RemoveMissingItemsWidget);
		const FText Message = FText::FromString(FString::Printf(TEXT("Missing items: %s"), *MissingItemsString));
		MissingItemsWidget = this->AAEquipment->CreateActionMessageOk(Message, MessageOk);
		this->AAEquipment->AddActionWidget(MissingItemsWidget);
	}
	else
		this->Done();
}

void AAALoadBlueprint::RemoveMissingItemsWidget()
{
	this->AAEquipment->RemoveActionWidget(MissingItemsWidget);
	MissingItemsWidget = nullptr;
}

void AAALoadBlueprint::EnterPickAnchor()
{
	bIsPickingAnchor = true;
	AAEquipment->CloseMainWidget();
}

void AAALoadBlueprint::EnterPlacing()
{
	ScrollUpInputActionBinding->bConsumeInput = true;
	ScrollDownInputActionBinding->bConsumeInput = true;
	bIsPlacing = true;
	AAEquipment->CloseMainWidget();
}