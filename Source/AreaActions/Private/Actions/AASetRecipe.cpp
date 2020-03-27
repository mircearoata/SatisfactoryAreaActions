// Fill out your copyright notice in the Description page of Project Settings.

#include "Actions/AASetRecipe.h"
#include "AAEquipment.h"
#include "FGBuildableManufacturer.h"

void AAASetRecipe::InternalRun() {
	if (!this->mSelectedRecipe) {
		return;
	}
	TMap<TSubclassOf<AFGBuildableManufacturer>, int> statistics;
	for (AActor* actor : this->mActors) {
		if (actor->IsA<AFGBuildableManufacturer>()) {
			if (UFGRecipe::GetProducedIn(this->mSelectedRecipe).Contains(actor->GetClass())) {
				((AFGBuildableManufacturer*)actor)->GetInputInventory()->Empty();
				((AFGBuildableManufacturer*)actor)->GetOutputInventory()->Empty();
				((AFGBuildableManufacturer*)actor)->SetRecipe(this->mSelectedRecipe);

				statistics.FindOrAdd(actor->GetClass())++;
			}
		}
	}
	if (statistics.Num() == 0) {
		FOnMessageOk messageOk;
		messageOk.BindDynamic(this, &AAASetRecipe::Done);
		FText title = FText::FromString(TEXT("Set Recipe"));
		FText message = FText::FromString(TEXT("No machines in the area accept this recipe."));
		this->mAAEquipment->ShowMessageOkDelegate(title, message, messageOk);
	}
	else {
		FString machinesCountString;
		for (auto& statisticsEntry : statistics) {
			machinesCountString += FString::Printf(TEXT("%d %s%s,"), statisticsEntry.Value, *((AFGBuildable*)statisticsEntry.Key->GetDefaultObject())->mDisplayName.ToString(), statisticsEntry.Value > 1 ? TEXT("s") : TEXT(""));
		}
		machinesCountString = machinesCountString.LeftChop(1);
		
		FOnMessageOk messageOk;
		messageOk.BindDynamic(this, &AAASetRecipe::Done);
		FText title = FText::FromString(TEXT("Set Recipe"));
		FText message = FText::FromString(FString::Printf(TEXT("Set recipe to %s for %s"), *UFGRecipe::GetRecipeName(this->mSelectedRecipe).ToString(), *machinesCountString));
		this->mAAEquipment->ShowMessageOkDelegate(title, message, messageOk);
	}
}