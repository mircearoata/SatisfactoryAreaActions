// Fill out your copyright notice in the Description page of Project Settings.

#include "Actions/AASetStationMode.h"
#include "AAEquipment.h"
#include "Buildables/FGBuildableDockingStation.h"
#include "Buildables/FGBuildableTrainPlatformCargo.h"

void AAASetStationMode::SetStationMode(const bool IsLoadMode)
{
    TMap<TSubclassOf<AFGBuildable>, int> Statistics;
    for (AActor* Actor : this->Actors)
    {
        if (AFGBuildableDockingStation* DockingStation = Cast<AFGBuildableDockingStation>(Actor))
        {
            DockingStation->SetIsInLoadMode(IsLoadMode);
            Statistics.FindOrAdd(Actor->GetClass())++;
        }
        if (AFGBuildableTrainPlatformCargo* TrainPlatform = Cast<AFGBuildableTrainPlatformCargo>(Actor))
        {
            TrainPlatform->SetIsInLoadMode(IsLoadMode);
            Statistics.FindOrAdd(Actor->GetClass())++;
        }
    }
    if (Statistics.Num() == 0)
    {
        FOnMessageOk MessageOk;
        MessageOk.BindDynamic(this, &AAASetStationMode::Done);
        const FText Message = FText::FromString(TEXT("No stations in the area."));
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
        MessageOk.BindDynamic(this, &AAASetStationMode::Done);
        const FText Message = FText::FromString(FString::Printf(TEXT("Set stations to %s for %s"),
            IsLoadMode ? TEXT("load") : TEXT("unload"),
            *MachinesCountString));
        UWidget* MessageOkWidget = this->AAEquipment->CreateActionMessageOk(Message, MessageOk);
        this->AAEquipment->AddActionWidget(MessageOkWidget);
    }
}
