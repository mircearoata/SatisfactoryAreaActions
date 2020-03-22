// Fill out your copyright notice in the Description page of Project Settings.


#include "AAIndicator.h"

void AAAIndicator::UpdateHeight(float minHeight, float maxHeight) {
	FVector newLocation = this->GetActorLocation();
	newLocation.Z = (minHeight + maxHeight) / 2;
	this->SetActorLocation(newLocation);

	FVector newScale = this->GetActorScale3D();
	newScale.Z = maxHeight - minHeight;
	this->SetActorScale3D(newScale);
}