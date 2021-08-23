// Fill out your copyright notice in the Description page of Project Settings.

#include "Actions/AACopy.h"

#include "AreaActionsModule.h"
#include "AABlueprintFunctionLibrary.h"
#include "AAEquipment.h"
#include "FGOutlineComponent.h"
#include "FGPlayerController.h"
#include "Buildables/FGBuildableStorage.h"

AAACopy::AAACopy() {
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
		if(AAEquipment->RaycastMouseWithRange(HitResult, true, true, true, HologramsActors))
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
}

void AAACopy::PrimaryFire()
{
	if(bIsPickingAnchor)
	{
		bIsPickingAnchor = false;
		FHitResult HitResult;
		if(AAEquipment->RaycastMouseWithRange(HitResult, true, true, true) && Actors.Contains(HitResult.Actor))
		{
			Anchor = static_cast<AFGBuildable*>(HitResult.Actor.Get());
		}
		else
		{
			Anchor = nullptr;
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

void AAACopy::EquipmentEquipped(AAAEquipment* Equipment)
{
	Super::EquipmentEquipped(Equipment);
	if(!InputComponent->HasBindings())
	{
		InputComponent->BindAction("PrimaryFire", EInputEvent::IE_Pressed, this, &AAACopy::PrimaryFire);
		InputComponent->BindAction("SecondaryFire", EInputEvent::IE_Pressed, this, &AAACopy::PrimaryFire);
		ScrollUpInputActionBinding = &InputComponent->BindAction("BuildGunScrollUp_PhotoModeFOVUp", EInputEvent::IE_Pressed, this, &AAACopy::ScrollUp);
		ScrollDownInputActionBinding = &InputComponent->BindAction("BuildGunScrollDown_PhotoModeFOVDown", EInputEvent::IE_Pressed, this, &AAACopy::ScrollDown);
		ScrollUpInputActionBinding->bConsumeInput = false;
		ScrollDownInputActionBinding->bConsumeInput = false;
	}
}

void AAACopy::Run_Implementation() {
	TArray<AFGBuildable*> BuildingsWithIssues;
	FText Error;
	if (!this->CopyBuildingsComponent->SetActors(this->Actors, BuildingsWithIssues, Error)) {
		if(!Error.IsEmpty())
		{
			FOnMessageOk MessageOk;
			MessageOk.BindDynamic(this, &AAACopy::Done);
			UWidget* MessageOkWidget = this->AAEquipment->CreateActionMessageOk(Error, MessageOk);
			this->AAEquipment->AddActionWidget(MessageOkWidget);
		}
		else
		{
			TArray<AActor*> AsActors;
			for (AFGBuildable* Building : BuildingsWithIssues) {
				AsActors.Add(Building);
			}
			UFGOutlineComponent::Get(GetWorld())->ShowDismantlePendingMaterial(AsActors);
			FOnMessageOk MessageOk;
			MessageOk.BindDynamic(this, &AAACopy::Done);
			const FText Message = FText::FromString(TEXT("Some buildings cannot be copied because they have connections to buildings outside the selected area. Remove the connections temporary, or include the connected buildings in the area. The buildings are highlighted."));
			UWidget* MessageOkWidget = this->AAEquipment->CreateActionMessageOk(Message, MessageOk);
			this->AAEquipment->AddActionWidget(MessageOkWidget);
		}
	}
	else {
		this->CopyBuildingsComponent->AddCopy(DeltaPosition, DeltaRotation, Anchor ? Anchor->GetActorLocation() : CopyBuildingsComponent->GetBuildingsCenter());
		this->ShowCopyWidget();
	}
}

void AAACopy::OnCancel_Implementation()
{
	this->CopyBuildingsComponent->RemoveCopy(0);
}

void AAACopy::Preview()
{
	this->CopyBuildingsComponent->MoveCopy(0, DeltaPosition, DeltaRotation, Anchor ? Anchor->GetActorLocation() : CopyBuildingsComponent->GetBuildingsCenter());
}

void AAACopy::Finish()
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
	if(!this->CopyBuildingsComponent->Finish(Inventories, MissingItems))
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
		MessageOk.BindDynamic(this, &AAACopy::RemoveMissingItemsWidget);
		const FText Message = FText::FromString(FString::Printf(TEXT("Missing items: %s"), *MissingItemsString));
		MissingItemsWidget = this->AAEquipment->CreateActionMessageOk(Message, MessageOk);
		this->AAEquipment->AddActionWidget(MissingItemsWidget);
	}
	else
		this->Done();
}

void AAACopy::RemoveMissingItemsWidget()
{
	this->AAEquipment->RemoveActionWidget(MissingItemsWidget);
	MissingItemsWidget = nullptr;
}

void AAACopy::EnterPickAnchor()
{
	bIsPickingAnchor = true;
	AAEquipment->CloseMainWidget();
}

void AAACopy::EnterPlacing()
{
	ScrollUpInputActionBinding->bConsumeInput = true;
	ScrollDownInputActionBinding->bConsumeInput = true;
	bIsPlacing = true;
	AAEquipment->CloseMainWidget();
}
