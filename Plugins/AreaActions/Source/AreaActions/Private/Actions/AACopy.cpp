// Fill out your copyright notice in the Description page of Project Settings.

#include "Actions/AACopy.h"

#include "AAEquipment.h"
#include "FGOutlineComponent.h"
#include "FGPlayerController.h"
#include "Buildables/FGBuildableStorage.h"

AAACopy::AAACopy() {
	this->CopyBuildingsComponent = CreateDefaultSubobject<UAACopyBuildingsComponent>(TEXT("CopyBuildings"));
	this->DeltaPosition = FVector::ZeroVector;
	this->DeltaRotation = FRotator::ZeroRotator;
	this->PreviewExists = false;
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
		this->ShowCopyWidget();
	}
}

void AAACopy::OnCancel_Implementation()
{
    if(PreviewExists)
	    this->CopyBuildingsComponent->RemoveCopy(0);
}

void AAACopy::Preview()
{
	if(!PreviewExists)
		this->CopyBuildingsComponent->AddCopy(DeltaPosition, DeltaRotation, IsRotationCenterSet ? RotationCenter : CopyBuildingsComponent->GetBuildingsCenter());
	else
		this->CopyBuildingsComponent->MoveCopy(0, DeltaPosition, DeltaRotation, IsRotationCenterSet ? RotationCenter : CopyBuildingsComponent->GetBuildingsCenter());
	PreviewExists = true;
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