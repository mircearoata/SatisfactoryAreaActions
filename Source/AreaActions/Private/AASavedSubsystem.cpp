// Fill out your copyright notice in the Description page of Project Settings.


#include "AASavedSubsystem.h"
#include "AASubsystemManager.h"

bool AAASavedSubsystem::ShouldSave_Implementation() const {
	return true;
}

AAASavedSubsystem* AAASavedSubsystem::Get(UObject* WorldContextObject) {
	return AAASubsystemManager::Get(WorldContextObject)->GetSavedSubsystem();
}