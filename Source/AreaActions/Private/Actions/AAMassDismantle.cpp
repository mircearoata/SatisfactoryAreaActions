// Fill out your copyright notice in the Description page of Project Settings.


#include "Actions/AAMassDismantle.h"
#include "FGBuildable.h"
#include "AAEquipment.h"
#include "FGCharacterPlayer.h"
#include "SML/util/Logging.h"

void AAAMassDismantle::InternalRun() {
	for (AActor* actor : this->mActors) {
		TArray<FInventoryStack> buildingRefunds;
		((AFGBuildable*)actor)->GetDismantleRefund_Implementation(buildingRefunds);
		((AFGBuildable*)actor)->Dismantle_Implementation();

		this->mRefunds.Append(buildingRefunds);
	}
	UFGInventoryLibrary::ConsolidateInventoryItems(this->mRefunds);
}

void AAAMassDismantle::GiveRefunds() {
	AFGCrate* crate = GetWorld()->SpawnActor<AFGCrate>(CrateClass, this->mAAEquipment->GetInstigatorCharacter()->GetActorLocation() + this->mAAEquipment->GetInstigatorCharacter()->GetActorForwardVector() * CrateDistance, FRotator(0, this->mAAEquipment->GetInstigatorCharacter()->GetActorRotation().Yaw + 90, 0));

	int crateInventorySlots = 0;
	for (FInventoryStack& stack : this->mRefunds) {
		int itemStackSize = UFGItemDescriptor::GetStackSize(stack.Item.ItemClass);
		crateInventorySlots += stack.NumItems / itemStackSize + !!(stack.NumItems % itemStackSize);
	}
	crate->GetInventory()->Resize(crateInventorySlots);
	crate->GetInventory()->AddStacks(this->mRefunds);
	this->mAAEquipment->ClearSelection();
	this->Done();
}