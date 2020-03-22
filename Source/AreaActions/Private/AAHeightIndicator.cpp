// Fill out your copyright notice in the Description page of Project Settings.


#include "AAHeightIndicator.h"

void AAAHeightIndicator::UpdateHeight(float minHeight, float maxHeight) {
	FVector newLocation = this->GetActorLocation();
	if (this->mIndicatorType == EAAHeightIndicatorType::TOP) {
		newLocation.Z = maxHeight;
	}
	else if (this->mIndicatorType == EAAHeightIndicatorType::BOTTOM) {
		newLocation.Z = minHeight;
	}
	this->SetActorLocation(newLocation);
}