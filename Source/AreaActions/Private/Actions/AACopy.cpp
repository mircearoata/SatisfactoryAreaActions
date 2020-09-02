// Fill out your copyright notice in the Description page of Project Settings.

#include "Actions/AACopy.h"

#include "AAEquipment.h"
#include "FGOutlineComponent.h"

AAACopy::AAACopy() {
	this->CopyBuildingsComponent = CreateDefaultSubobject<UAACopyBuildingsComponent>(TEXT("CopyBuildings"));
	this->DeltaPosition = FVector::ZeroVector;
	this->DeltaRotation = FRotator::ZeroRotator;
	this->PreviewExists = false;
}

void AAACopy::Run_Implementation() {
	TArray<AFGBuildable*> BuildingsWithIssues;
	if (!this->CopyBuildingsComponent->SetActors(this->Actors, BuildingsWithIssues)) {
		TArray<AActor*> AsActors;
		for (AFGBuildable* Building : BuildingsWithIssues) {
			AsActors.Add(Building);
		}
		UFGOutlineComponent::Get(GetWorld())->ShowDismantlePendingMaterial(AsActors);
		FOnMessageOk MessageOk;
		MessageOk.BindDynamic(this, &AAACopy::Done);
		const FText Message = FText::FromString(TEXT("Some buildings cannot be copied because they have connections to buildings outside the selected area. Remove the connections temporary, or include the connected buildings in the area. The buildings are highlighted."));
		UWidget* MessageOkWidget = this->AAEquipment->CreateActionMessageOk(Message, MessageOk);
		this->AAEquipment->AddActionWidget(MessageOkWidget);
	}
	else {
		this->ShowCopyWidget();
	}
}

void AAACopy::OnCancel_Implementation()
{
	this->CopyBuildingsComponent->RemoveCopy(0);
}

void AAACopy::Preview()
{
	if(!PreviewExists)
		this->CopyBuildingsComponent->AddCopy(DeltaPosition, DeltaRotation, IsRotationCenterSet ? RotationCenter : CopyBuildingsComponent->GetBuildingsCenter());
	else
		this->CopyBuildingsComponent->MoveCopy(0, DeltaPosition, DeltaRotation, IsRotationCenterSet ? RotationCenter : CopyBuildingsComponent->GetBuildingsCenter());
	PreviewExists = true;
}

void AAACopy::Finish()
{
	this->Preview();
	this->CopyBuildingsComponent->Finish();
	this->Done();
}
