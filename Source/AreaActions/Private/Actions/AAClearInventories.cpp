// Fill out your copyright notice in the Description page of Project Settings.

#include "Actions/AAClearInventories.h"

#include "FGBuildable.h"
#include "AAEquipment.h"
#include "FGCharacterPlayer.h"
#include "FGBuildableConveyorBase.h"
#include "FGBuildableManufacturer.h"
#include "SML/util/Logging.h"

void AAAClearInventories::Run_Implementation()
{
    FOnMessageYes MessageYes;
    MessageYes.BindDynamic(this, &AAAClearInventories::ClearInventories);
    FOnMessageNo MessageNo;
    MessageNo.BindDynamic(this, &AAAClearInventories::Done);
    const FText Message = FText::FromString(FString::Printf(TEXT("Are you sure you want to clear the inventories of %d buildings?"), Actors.Num()));
    this->AAEquipment->ShowMessageYesNoDelegate(ActionName, Message, MessageYes, MessageNo);
}

void AAAClearInventories::ClearInventories()
{
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
                ConveyorBase->mItems.FlagForRemoveAt(i);
            ConveyorBase->mPendingUpdateItemTransforms = true;
        }
    }
    
    FOnMessageOk MessageOk;
    MessageOk.BindDynamic(this, &AAAClearInventories::Done);
    const FText Message = FText::FromString(FString::Printf(TEXT("Cleared %d items"), ItemCount));
    this->AAEquipment->ShowMessageOkDelegate(ActionName, Message, MessageOk);
}