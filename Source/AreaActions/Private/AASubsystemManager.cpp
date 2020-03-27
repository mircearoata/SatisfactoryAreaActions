// Fill out your copyright notice in the Description page of Project Settings.

#include "AASubsystemManager.h"
#include "AAInitMod.h"

bool AAASubsystemManager::ShouldSave_Implementation() const {
	return true;
}

void AAASubsystemManager::GatherDependencies_Implementation(TArray< UObject* >& out_dependentObjects) {
	out_dependentObjects.Add(mSavedSubsystem);
}

void AAASubsystemManager::BeginPlay() {
	if (!mSavedSubsystem) {
		mSavedSubsystem = GetWorld()->SpawnActor<AAASavedSubsystem>(SavedSubsystemClass, FVector::ZeroVector, FRotator::ZeroRotator);
	}
	if (!mActionsManager) {
		mActionsManager = GetWorld()->SpawnActor<AAAActionsManager>(ActionsManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
	}
}

AAASubsystemManager* AAASubsystemManager::Get(UObject* WorldContextObject) {
	return AAAInitMod::Get(WorldContextObject)->GetSubsystemManager();
}
