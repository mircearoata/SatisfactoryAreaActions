// Fill out your copyright notice in the Description page of Project Settings.

#include "AAActionsManager.h"
#include "AASubsystemManager.h"

AAAActionsManager* AAAActionsManager::Get(UObject* WorldContextObject) {
	return AAASubsystemManager::Get(WorldContextObject)->GetActionsManager();
}

void AAAActionsManager::GetAvailableActions(TArray<TSubclassOf<AAAAction>>& OutAvailableActions) const
{
	OutAvailableActions = this->AvailableActions;
}

void AAAActionsManager::GetAvailableActionsInCategory(TSubclassOf<UAAActionCategory> ActionCategory,
	TArray<TSubclassOf<AAAAction>>& OutAvailableActions) const
{
	for(TSubclassOf<AAAAction> Action : this->AvailableActions)
		if(AAAAction::GetActionCategory(Action) == ActionCategory)
			OutAvailableActions.Add(Action);
}
