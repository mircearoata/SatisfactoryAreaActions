// Fill out your copyright notice in the Description page of Project Settings.

#include "AAIndicator.h"

void AAAIndicator::UpdateHeight(const float MinHeight, const float MaxHeight) {
	FVector NewLocation = this->GetActorLocation();
	NewLocation.Z = (MinHeight + MaxHeight) / 2;
	this->SetActorLocation(NewLocation);

	FVector NewScale = this->GetActorScale3D();
	NewScale.Z = MaxHeight - MinHeight;
	this->SetActorScale3D(NewScale);
}