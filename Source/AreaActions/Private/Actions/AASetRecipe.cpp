// Fill out your copyright notice in the Description page of Project Settings.

#include "Actions/AASetRecipe.h"
#include "Buildables/FGBuildableManufacturer.h"

bool ManufacturerAcceptsRecipe(TSubclassOf<AFGBuildableManufacturer> ManufacturerClass, TSubclassOf<UFGRecipe> Recipe)
{
    TArray<TSubclassOf<UObject>> ProducedIn = UFGRecipe::GetProducedIn(Recipe);
    for(TSubclassOf<UObject> Producer : ProducedIn)
    {
        if((*ManufacturerClass)->IsChildOf(*Producer))
            return true;
    }
    return false;
}

void AAASetRecipe::SetRecipe(const TSubclassOf<UFGRecipe> SelectedRecipe, TMap<TSubclassOf<AFGBuildableManufacturer>, int>& Statistics)
{
    if (!SelectedRecipe)
    {
        this->Done();
        return;
    }

    for (AActor* Actor : this->Actors)
    {
        if (AFGBuildableManufacturer* Manufacturer = Cast<AFGBuildableManufacturer>(Actor))
        {
            if (ManufacturerAcceptsRecipe(Manufacturer->GetClass(), SelectedRecipe))
            {
                Manufacturer->GetInputInventory()->Empty();
                Manufacturer->GetOutputInventory()->Empty();
                Manufacturer->SetRecipe(SelectedRecipe);

                Statistics.FindOrAdd(Actor->GetClass())++;
            }
        }
    }
}
