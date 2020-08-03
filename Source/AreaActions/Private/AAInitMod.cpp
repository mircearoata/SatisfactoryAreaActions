// Fill out your copyright notice in the Description page of Project Settings.

#include "AAInitMod.h"

AAAInitMod* AAAInitMod::Get(UObject* WorldContextObject) {
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject, AAAInitMod::StaticClass(), Actors);
	return static_cast<AAAInitMod*>(Actors[0]);
}

void AAAInitMod::InitSubsystemManager() {
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(this, AAASubsystemManager::StaticClass(), Actors);
	if (Actors.Num() == 0) {
		this->SubsystemManager = GetWorld()->SpawnActor<AAASubsystemManager>(SubsystemManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
	}
	else {
		this->SubsystemManager = static_cast<AAASubsystemManager*>(Actors[0]);
	}
}
