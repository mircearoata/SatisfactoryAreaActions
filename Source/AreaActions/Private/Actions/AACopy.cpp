// Fill out your copyright notice in the Description page of Project Settings.

#include "Actions/AACopy.h"
#include "AAEquipment.h"
#include "FGOutlineComponent.h"

AAACopy::AAACopy() {
	this->mCopyBuildingsComponent = CreateDefaultSubobject<UAACopyBuildingsComponent>(TEXT("CopyBuildings"));
}

void AAACopy::Run_Implementation() {
	this->mCopyBuildingsComponent->SetAAEquipment(this->mAAEquipment);
	TArray<AFGBuildable*> buildingsWithIssues;
	if (!this->mCopyBuildingsComponent->SetActors(this->mActors, buildingsWithIssues)) {
		TArray<AActor*> asActors;
		for (AFGBuildable* building : buildingsWithIssues) {
			asActors.Add(building);
		}
		UFGOutlineComponent::Get(GetWorld())->ShowDismantlePendingMaterial(asActors);
		FOnMessageOk messageOk;
		messageOk.BindDynamic(this, &AAACopy::Done);
		FText message = FText::FromString(TEXT("Some buildings cannot be copied because they have connections to buildings outside the selected area. Remove the connections temporary, or include the connected buildings in the area. The buildings are highlighted."));
		this->mAAEquipment->ShowMessageOkDelegate(ActionName, message, messageOk);
	}
	else {
		this->mCopyBuildingsComponent->AddCopy(FVector(0, 0, 1000), FRotator::ZeroRotator);
		FTimerDelegate TimerCallback;
		TimerCallback.BindLambda([=]()
        {
			this->mCopyBuildingsComponent->Finish();
			this->Done();
        });

		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, TimerCallback, 10.0f, false);
	}
}