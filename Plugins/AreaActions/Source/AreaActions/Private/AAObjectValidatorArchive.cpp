// Fill out your copyright notice in the Description page of Project Settings.

#include "AAObjectValidatorArchive.h"

#include "FGSaveInterface.h"
#include "SaveCustomVersion.h"
#include "GameFramework/Actor.h"
// #include "util/Logging.h"

FAAObjectValidatorArchive::FAAObjectValidatorArchive(TArray<UObject*>& InAllObjects) : AllObjects(InAllObjects)
{
    SetIsSaving(true);
    ArIsSaveGame = true;
    UsingCustomVersion(FSaveCustomVersion::GUID);
    this->CurrentObject = nullptr;
}

bool FAAObjectValidatorArchive::Validate(UObject* Object)
{
    Valid = true;
    CurrentObject = Object;
    Object->Serialize(*this);
    return Valid;
}

bool FAAObjectValidatorArchive::IsValid(UObject* Object) const
{
    if(!CurrentObject) return true;
    if(AActor* Actor = Cast<AActor>(CurrentObject))
    {
        if(Actor->GetOwner() == Object) return true;
    }
    return AllObjects.Contains(Object);
}

void FAAObjectValidatorArchive::SetInvalid(UObject* InvalidObject)
{
    // SML::Logging::debug(*CurrentObject->GetPathName(), TEXT(" -> "), *InvalidObject->GetPathName());
    Valid = false;
}

FArchive& FAAObjectValidatorArchive::operator<<(UObject*& Value)
{
    if(Value && Value->Implements<UFGSaveInterface>() && !IsValid(Value))
        SetInvalid(Value);
    return *this;
}

FArchive& FAAObjectValidatorArchive::operator<<(FLazyObjectPtr& Value)
{
    if (Value.IsValid() && Value.Get() && Value.Get()->Implements<UFGSaveInterface>() && !IsValid(Value.Get()))
        SetInvalid(Value.Get());
    return *this;
}

FArchive& FAAObjectValidatorArchive::operator<<(FSoftObjectPtr& Value)
{
    if (Value.IsValid() && Value.Get() && Value.Get()->Implements<UFGSaveInterface>() && !IsValid(Value.Get()))
        SetInvalid(Value.Get());
    return *this;
}

FArchive& FAAObjectValidatorArchive::operator<<(FSoftObjectPath& Value)
{
    return *this;
}

FArchive& FAAObjectValidatorArchive::operator<<(FWeakObjectPtr& Value)
{
    if (Value.IsValid() && Value.Get() && Value.Get()->Implements<UFGSaveInterface>() && !IsValid(Value.Get()))
        SetInvalid(Value.Get());
    return *this;
}
