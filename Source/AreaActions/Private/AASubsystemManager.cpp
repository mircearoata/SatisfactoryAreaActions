// Fill out your copyright notice in the Description page of Project Settings.

#include "AASubsystemManager.h"
#include "AAInitMod.h"

bool AAASubsystemManager::ShouldSave_Implementation() const {
	return true;
}

void AAASubsystemManager::GatherDependencies_Implementation(TArray< UObject* >& OutDependentObjects) {
	OutDependentObjects.Add(SavedSubsystem);
}

void AAASubsystemManager::BeginPlay() {
	if (!SavedSubsystem) {
		SavedSubsystem = GetWorld()->SpawnActor<AAASavedSubsystem>(SavedSubsystemClass, FVector::ZeroVector, FRotator::ZeroRotator);
	}
	if (!ActionsManager) {
		ActionsManager = GetWorld()->SpawnActor<AAAActionsManager>(ActionsManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
	}
}

AAASubsystemManager* AAASubsystemManager::Get(UObject* WorldContextObject) {
	return AAAInitMod::Get(WorldContextObject)->GetSubsystemManager();
}
