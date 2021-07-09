// Fill out your copyright notice in the Description page of Project Settings.

#include "AAGameWorldModule.h"

#include "Kismet/GameplayStatics.h"
#include "ModLoading/ModLoadingLibrary.h"
#include "Module/WorldModuleManager.h"

UAAGameWorldModule* UAAGameWorldModule::Get(UObject* WorldContextObject) {
#if !WITH_EDITOR
	UWorldModuleManager* ModuleManager = GWorld->GetSubsystem<UWorldModuleManager>();
	UAAGameWorldModule* WorldModule = Cast<UAAGameWorldModule>(ModuleManager->FindModule(FName(TEXT("AreaActions"))));
	return WorldModule;
#else
	TSubclassOf<UAAGameWorldModule> Class = FSoftClassPath(TEXT("/AreaActions/AAGameWorldModule.AAGameWorldModule_C")).TryLoadClass<UAAGameWorldModule>();
	UAAGameWorldModule* WorldModule = NewObject<UAAGameWorldModule>(WorldContextObject->GetWorld(), Class);
	WorldModule->InitSubsystemManager();
	return WorldModule;
#endif
}

void UAAGameWorldModule::InitSubsystemManager() {
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(this, AAASubsystemManager::StaticClass(), Actors);
	if (Actors.Num() == 0) {
		this->SubsystemManager = GetWorld()->SpawnActor<AAASubsystemManager>(SubsystemManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
	}
	else {
		this->SubsystemManager = static_cast<AAASubsystemManager*>(Actors[0]);
	}
}

void UAAGameWorldModule::GetAreaActionsVersion(FVersion& Version)
{
	FModInfo ModInfo;
	GEngine->GetEngineSubsystem<UModLoadingLibrary>()->GetLoadedModInfo(TEXT("AreaActions"), ModInfo);
	Version = ModInfo.Version;
#if AA_DEBUG
	Version.Type += "+DEV";
#endif
}

void UAAGameWorldModule::DispatchLifecycleEvent(ELifecyclePhase Phase)
{
	Super::DispatchLifecycleEvent(Phase);
	InitSubsystemManager();
}
