// Fill out your copyright notice in the Description page of Project Settings.

#include "AAHeightIndicator.h"

void AAAHeightIndicator::UpdateHeight(const float MinHeight, const float MaxHeight) {
	FVector NewLocation = this->GetActorLocation();
	if (this->IndicatorType == Top) {
		NewLocation.Z = MaxHeight;
	}
	else if (this->IndicatorType == Bottom) {
		NewLocation.Z = MinHeight;
	}
	this->SetActorLocation(NewLocation);
}