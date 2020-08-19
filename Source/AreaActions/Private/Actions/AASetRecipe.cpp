// Fill out your copyright notice in the Description page of Project Settings.

#include "Actions/AASetRecipe.h"
#include "AAEquipment.h"
#include "FGBuildableManufacturer.h"

void AAASetRecipe::SetRecipe(const TSubclassOf<UFGRecipe> SelectedRecipe)
{
    if (!SelectedRecipe)
    {
        this->Done();
        return;
    }
    TMap<TSubclassOf<AFGBuildableManufacturer>, int> Statistics;
    for (AActor* Actor : this->Actors)
    {
        if (Actor->IsA<AFGBuildableManufacturer>())
        {
            if (UFGRecipe::GetProducedIn(SelectedRecipe).Contains(Actor->GetClass()))
            {
                static_cast<AFGBuildableManufacturer*>(Actor)->GetInputInventory()->Empty();
                static_cast<AFGBuildableManufacturer*>(Actor)->GetOutputInventory()->Empty();
                static_cast<AFGBuildableManufacturer*>(Actor)->SetRecipe(SelectedRecipe);

                Statistics.FindOrAdd(Actor->GetClass())++;
            }
        }
    }
    if (Statistics.Num() == 0)
    {
        FOnMessageOk MessageOk;
        MessageOk.BindDynamic(this, &AAASetRecipe::Done);
        const FText Message = FText::FromString(TEXT("No machines in the area accept this recipe."));
        UWidget* MessageOkWidget = this->AAEquipment->CreateActionMessageOk(Message, MessageOk);
        this->AAEquipment->AddActionWidget(MessageOkWidget);
    }
    else
    {
        FString MachinesCountString;
        for (auto& StatisticsEntry : Statistics)
        {
            MachinesCountString += FString::Printf(TEXT("%d %s%s,"), StatisticsEntry.Value,
                                                   *static_cast<AFGBuildable*>(StatisticsEntry.Key->GetDefaultObject())->
                                                    mDisplayName.ToString(),
                                                   StatisticsEntry.Value > 1 ? TEXT("s") : TEXT(""));
        }
        MachinesCountString = MachinesCountString.LeftChop(1);

        FOnMessageOk MessageOk;
        MessageOk.BindDynamic(this, &AAASetRecipe::Done);
        const FText Message = FText::FromString(FString::Printf(
            TEXT("Set recipe to %s for %s"), *UFGRecipe::GetRecipeName(SelectedRecipe).ToString(),
            *MachinesCountString));
        UWidget* MessageOkWidget = this->AAEquipment->CreateActionMessageOk(Message, MessageOk);
        this->AAEquipment->AddActionWidget(MessageOkWidget);
    }
}
