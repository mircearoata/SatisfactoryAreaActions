// Fill out your copyright notice in the Description page of Project Settings.

#include "Actions/AAClearInventories.h"

#include "Buildables/FGBuildable.h"
#include "AAEquipment.h"
#include "FGCharacterPlayer.h"
#include "Buildables/FGBuildableConveyorBase.h"
#include "Buildables/FGBuildableManufacturer.h"

void AAAClearInventories::Run_Implementation()
{
    FOnMessageYes MessageYes;
    MessageYes.BindDynamic(this, &AAAClearInventories::ClearInventories);
    FOnMessageNo MessageNo;
    MessageNo.BindDynamic(this, &AAAClearInventories::Done);
    const FText Message = FText::FromString(FString::Printf(TEXT("Are you sure you want to clear the inventories of %d buildings?"), Actors.Num()));
    ConfirmWidget = this->AAEquipment->CreateActionMessageYesNo(Message, MessageYes, MessageNo);
    this->AAEquipment->AddActionWidget(ConfirmWidget);
}

void AAAClearInventories::ClearInventories()
{
    this->AAEquipment->RemoveActionWidget(ConfirmWidget);
    int32 ItemCount = 0;
    for (AActor* Actor : this->Actors)
    {
        if(AFGBuildableFactory* FactoryBuilding = Cast<AFGBuildableFactory>(Actor))
        {
            TArray<UFGInventoryComponent*> InventoryComponents;
            FactoryBuilding->GetComponents<UFGInventoryComponent>(InventoryComponents);
            for(UFGInventoryComponent* InventoryComponent : InventoryComponents)
            {
                if(InventoryComponent != FactoryBuilding->GetPotentialInventory())
                {
                    for(int i = 0; i < InventoryComponent->GetSizeLinear(); i++)
                    {
                        FInventoryStack Stack;
                        InventoryComponent->GetStackFromIndex(i, Stack);
                        ItemCount += Stack.NumItems;
                        InventoryComponent->RemoveAllFromIndex(i);
                    }
                }
            }
        }

        if(AFGBuildableConveyorBase* ConveyorBase = Cast<AFGBuildableConveyorBase>(Actor))
        {
            ItemCount += ConveyorBase->mItems.Num();
            for(int i = ConveyorBase->mItems.Num() - 1; i >= 0; i--)
                if(ConveyorBase->mItems.IsValidIndex(i))
                    ConveyorBase->Factory_RemoveItemAt(i);
        }
    }
    
    FOnMessageOk MessageOk;
    MessageOk.BindDynamic(this, &AAAClearInventories::Done);
    const FText Message = FText::FromString(FString::Printf(TEXT("Cleared %d items"), ItemCount));
    UWidget* MessageOkWidget = this->AAEquipment->CreateActionMessageOk(Message, MessageOk);
    this->AAEquipment->AddActionWidget(MessageOkWidget);
}