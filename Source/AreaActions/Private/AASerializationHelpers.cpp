// Fill out your copyright notice in the Description page of Project Settings.


#include "AASerializationHelpers.h"

#include "FGSaveInterface.h"
#include "FGSaveSession.h"

void UAASerializationHelpers::CallPreSaveGame(UObject* Object)
{
	if (Object->Implements<UFGSaveInterface>())
		IFGSaveInterface::Execute_PreSaveGame(
			Object, UFGSaveSession::GetVersion(UFGSaveSession::Get(Object->GetWorld())->mSaveHeader),
			FEngineVersion::Current().GetChangelist());
}

void UAASerializationHelpers::CallPostSaveGame(UObject* Object)
{
	if (Object->Implements<UFGSaveInterface>())
		IFGSaveInterface::Execute_PostSaveGame(
			Object, UFGSaveSession::GetVersion(UFGSaveSession::Get(Object->GetWorld())->mSaveHeader),
			FEngineVersion::Current().GetChangelist());
}

void UAASerializationHelpers::CallPreLoadGame(UObject* Object)
{
	if (Object->Implements<UFGSaveInterface>())
		IFGSaveInterface::Execute_PreLoadGame(
			Object, UFGSaveSession::GetVersion(UFGSaveSession::Get(Object->GetWorld())->mSaveHeader),
			FEngineVersion::Current().GetChangelist());
}

void UAASerializationHelpers::CallPostLoadGame(UObject* Object)
{
	if (Object->Implements<UFGSaveInterface>())
		IFGSaveInterface::Execute_PostLoadGame(
			Object, UFGSaveSession::GetVersion(UFGSaveSession::Get(Object->GetWorld())->mSaveHeader),
			FEngineVersion::Current().GetChangelist());
}
