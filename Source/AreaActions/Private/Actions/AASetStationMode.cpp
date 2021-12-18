// Fill out your copyright notice in the Description page of Project Settings.

#include "Actions/AASetStationMode.h"
#include "Buildables/FGBuildableDockingStation.h"
#include "Buildables/FGBuildableTrainPlatformCargo.h"

void AAASetStationMode::SetStationMode(const bool IsLoadMode, TMap<TSubclassOf<AFGBuildable>, int>& Statistics)
{
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
}
