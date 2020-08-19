// Fill out your copyright notice in the Description page of Project Settings.

#include "Actions/AAMassDismantle.h"

#include "FGBuildable.h"
#include "AAEquipment.h"
#include "FGCharacterPlayer.h"
#include "FGInventoryLibrary.h"
#include "SML/util/Logging.h"

void AAAMassDismantle::Run_Implementation()
{
    FOnMessageYes MessageYes;
    MessageYes.BindDynamic(this, &AAAMassDismantle::Dismantle);
    FOnMessageNo MessageNo;
    MessageNo.BindDynamic(this, &AAAMassDismantle::Done);
    const FText Message = FText::FromString(FString::Printf(TEXT("Are you sure you want to dismantle %d buildings?\nThe game will freeze for a while."), Actors.Num()));
    ConfirmWidget = this->AAEquipment->CreateActionMessageYesNo(Message, MessageYes, MessageNo);
    this->AAEquipment->AddActionWidget(ConfirmWidget);
}

void AAAMassDismantle::Dismantle()
{
    this->AAEquipment->RemoveActionWidget(ConfirmWidget);
    for (AActor* Actor : this->Actors)
    {
        TArray<FInventoryStack> BuildingRefunds;
        static_cast<AFGBuildable*>(Actor)->GetDismantleRefund_Implementation(BuildingRefunds);
        static_cast<AFGBuildable*>(Actor)->Dismantle_Implementation();

        this->Refunds.Append(BuildingRefunds);
    }
    UFGInventoryLibrary::ConsolidateInventoryItems(this->Refunds);

    int32 ItemCount = 0;
    for(const FInventoryStack Stack : this->Refunds)
        ItemCount += Stack.NumItems;
    
    FOnMessageYes MessageYes;
    MessageYes.BindDynamic(this, &AAAMassDismantle::GiveRefunds);
    FOnMessageNo MessageNo;
    MessageNo.BindDynamic(this, &AAAMassDismantle::Done);
    const FText Message = FText::FromString(FString::Printf(TEXT("Do you want to receive the refunds (%d items) in a crate near you?"), ItemCount));
    RefundsWidget = this->AAEquipment->CreateActionMessageYesNo(Message, MessageYes, MessageNo);
    this->AAEquipment->AddActionWidget(RefundsWidget);
}

void AAAMassDismantle::GiveRefunds()
{
    this->AAEquipment->RemoveActionWidget(RefundsWidget);
    AFGCrate* Crate = GetWorld()->SpawnActor<AFGCrate>(
        CrateClass,
        this->AAEquipment->GetInstigatorCharacter()->GetActorLocation() + this->AAEquipment->GetInstigatorCharacter()->GetActorForwardVector() * CrateDistance,
        FRotator(0, this->AAEquipment->GetInstigatorCharacter()->GetActorRotation().Yaw + 90, 0));

    int CrateInventorySlots = 0;
    for (FInventoryStack& Stack : this->Refunds)
    {
        const int ItemStackSize = UFGItemDescriptor::GetStackSize(Stack.Item.ItemClass);
        CrateInventorySlots += Stack.NumItems / ItemStackSize + !!(Stack.NumItems % ItemStackSize);
    }
    Crate->GetInventory()->Resize(CrateInventorySlots);
    Crate->GetInventory()->AddStacks(this->Refunds);
    this->AAEquipment->ClearSelection();
    this->Done();
}
