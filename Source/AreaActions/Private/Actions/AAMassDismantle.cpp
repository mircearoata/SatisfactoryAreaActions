// Fill out your copyright notice in the Description page of Project Settings.


#include "Actions/AAMassDismantle.h"
#include "FGBuildable.h"
#include "AAEquipment.h"
#include "FGCharacterPlayer.h"
#include "SML/util/Logging.h"
#include "FGInventoryLibrary.h"

void AAAMassDismantle::Run() {
	TArray<FInventoryStack> refunds;
	for (AActor* actor : this->mActors) {
		TArray<FInventoryStack> buildingRefunds;
		((AFGBuildable*)actor)->GetDismantleRefund_Implementation(buildingRefunds);
		((AFGBuildable*)actor)->Dismantle_Implementation();

		refunds.Append(buildingRefunds);
	}
	UFGInventoryLibrary::ConsolidateInventoryItems(refunds);
	AFGCrate* crate = GetWorld()->SpawnActor<AFGCrate>(CrateClass, this->mAAEquipment->GetInstigatorCharacter()->GetActorLocation() + this->mAAEquipment->GetInstigatorCharacter()->GetActorForwardVector() * CrateDistance, FRotator(0, this->mAAEquipment->GetInstigatorCharacter()->GetActorRotation().Yaw + 90, 0));

	int crateInventorySlots = 0;
	for (FInventoryStack& stack : refunds) {
		int itemStackSize = UFGItemDescriptor::GetStackSize(stack.Item.ItemClass);
		crateInventorySlots += stack.NumItems / itemStackSize + !!(stack.NumItems % itemStackSize);
	}
	crate->GetInventory()->Resize(crateInventorySlots);
	crate->GetInventory()->AddStacks(refunds);
}
