// Fill out your copyright notice in the Description page of Project Settings.

#include "AAHeightIndicator.h"

void AAAHeightIndicator::UpdateHeight(float minHeight, float maxHeight) {
	FVector newLocation = this->GetActorLocation();
	if (this->mIndicatorType == HIT_TOP) {
		newLocation.Z = maxHeight;
	}
	else if (this->mIndicatorType == HIT_BOTTOM) {
		newLocation.Z = minHeight;
	}
	this->SetActorLocation(newLocation);
}