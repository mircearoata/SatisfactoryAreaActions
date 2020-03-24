// Fill out your copyright notice in the Description page of Project Settings.


#include "AAInitMod.h"

AAAInitMod* AAAInitMod::Get(UObject* WorldContextObject) {
	TArray<AActor*> actors;
	UGameplayStatics::GetAllActorsOfClass(WorldContextObject, AAAInitMod::StaticClass(), actors);
	return (AAAInitMod*)actors[0];
}

void AAAInitMod::InitSubsystemManager() {
	TArray<AActor*> actors;
	UGameplayStatics::GetAllActorsOfClass(this, AAASubsystemManager::StaticClass(), actors);
	if (actors.Num() == 0) {
		this->mSubsystemManager = GetWorld()->SpawnActor<AAASubsystemManager>(SubsystemManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
	}
	else {
		this->mSubsystemManager = (AAASubsystemManager*)actors[0];
	}
}
