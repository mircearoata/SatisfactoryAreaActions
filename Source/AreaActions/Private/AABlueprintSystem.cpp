// Fill out your copyright notice in the Description page of Project Settings.


#include "AABlueprintSystem.h"

UAABlueprintSystem::UAABlueprintSystem() : Super()
{
}

void UAABlueprintSystem::Initialize(FSubsystemCollectionBase& MovieSceneBlends)
{
	Super::Initialize(MovieSceneBlends);
	DiscoverBlueprints();
}

void UAABlueprintSystem::DiscoverBlueprints()
{
	const FString BlueprintsDir = GetBlueprintsDir();
	TArray<FString> FileNames;
	IFileManager::Get().FindFiles(FileNames, *BlueprintsDir, TEXT(".aabp"));
	CachedBlueprints.Empty();
	for(FString FileName : FileNames)
	{
		FString FilePath = BlueprintsDir / FileName;
		TArray<uint8> FileRaw;
		if (!FFileHelper::LoadFileToArray(FileRaw, *FilePath))
			continue;
		FMemoryReader MemoryReader(FileRaw);
		FObjectAndNameAsStringProxyArchive MemoryReaderProxy(MemoryReader, true);
		FAABlueprintHeader Header;
		MemoryReaderProxy << Header;
		if(!MemoryReaderProxy.IsError())
			CachedBlueprints.Add(FilePath, Header);
	}
}

bool UAABlueprintSystem::SaveBlueprintInternal(const FString FilePath, UAABlueprint* Blueprint) const
{
	TArray<uint8> FileRaw;
	FMemoryWriter MemoryWriter(FileRaw);
	FObjectAndNameAsStringProxyArchive MemoryWriterProxy(MemoryWriter, true);
	Blueprint->SerializeBlueprint(MemoryWriterProxy);
	return !MemoryWriter.IsError() && FFileHelper::SaveArrayToFile(FileRaw, *FilePath);
}

bool UAABlueprintSystem::SaveBlueprint(const FString FilePath, UAABlueprint* Blueprint)
{
	const bool Result = SaveBlueprintInternal(FilePath, Blueprint);
	if(Result)
	{
		CachedBlueprints.Add(FilePath, Blueprint->BlueprintHeader);
	}
	return Result;
}

UAABlueprint* UAABlueprintSystem::LoadBlueprint(const FString FilePath)
{
	UAABlueprint* Blueprint = NewObject<UAABlueprint>(this);
	TArray<uint8> FileRaw;
	if (!FFileHelper::LoadFileToArray(FileRaw, *FilePath))
		return nullptr;
	FMemoryReader MemoryReader(FileRaw);
	FObjectAndNameAsStringProxyArchive MemoryReaderProxy(MemoryReader, true);
	Blueprint->SerializeBlueprint(MemoryReaderProxy);
	if(MemoryReader.IsError())
		return nullptr;
	return Blueprint;
}

FString UAABlueprintSystem::GetBlueprintsDir()
{
	return FPaths::ProjectSavedDir() / TEXT("AreaActionsBlueprints");
}

FString UAABlueprintSystem::GetBlueprintPath(const FString Name)
{
	return GetBlueprintsDir() / FString::Printf(TEXT("%s.aabp"), *Name);
}
