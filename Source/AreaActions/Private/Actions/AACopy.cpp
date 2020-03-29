// Fill out your copyright notice in the Description page of Project Settings.

#include "Actions/AACopy.h"

AAACopy::AAACopy() {
	this->mCopyBuildingsComponent = CreateDefaultSubobject<UAACopyBuildingsComponent>(TEXT("CopyBuildings"));
}

void AAACopy::Run_Implementation() {
	this->mCopyBuildingsComponent->SetAAEquipment(this->mAAEquipment);
	this->mCopyBuildingsComponent->SetActors(this->mActors);
	this->mCopyBuildingsComponent->AddCopy(FVector(0, 0, 1000), FRotator::ZeroRotator);
	this->Done();
}