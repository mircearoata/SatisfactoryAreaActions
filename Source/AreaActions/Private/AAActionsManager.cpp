// Fill out your copyright notice in the Description page of Project Settings.


#include "AAActionsManager.h"
#include "AASubsystemManager.h"

AAAActionsManager* AAAActionsManager::Get(UObject* WorldContextObject) {
	return AAASubsystemManager::Get(WorldContextObject)->GetActionsManager();
}