// Fill out your copyright notice in the Description page of Project Settings.

#include "AAObjectCollectorArchive.h"

#include "FGSaveInterface.h"
#include "SaveCustomVersion.h"
#include "GameFramework/Actor.h"

FAAObjectCollectorArchive::FAAObjectCollectorArchive(TArray<UObject*>* AllObjects)
{
    SetIsSaving(true);
    ArIsSaveGame = true;
    UsingCustomVersion(FSaveCustomVersion::GUID);
    this->AllObjects = AllObjects;
    this->CurrentObject = nullptr;
}

void FAAObjectCollectorArchive::GetAllObjects(TArray<UObject*>& Root)
{
    RootObjects = Root;
    for(UObject* Object : Root)
    {
        CurrentObject = Object;
        Object->Serialize(*this);
        this->AllObjects->AddUnique(Object);
    }
}

void FAAObjectCollectorArchive::AddObject(UObject* Object)
{
    if(!this->AllObjects->Contains(Object))
    {
        this->AllObjects->Add(Object);
        UObject* Parent = Object;
        while(Parent)
        {
            if(this->RootObjects.Contains(Parent))
            {
                Object->Serialize(*this);
                break;
            }
            Parent = Parent->GetOuter();
        }
    }
}

bool FAAObjectCollectorArchive::ShouldSave(UObject* Object) const
{
    if(!CurrentObject) return true;
    if(AActor* Actor = Cast<AActor>(CurrentObject))
    {
        if(Actor->GetOwner() == Object) return false;
    }
    if(AActor* Actor = Cast<AActor>(Object))
    {
        if(!RootObjects.Contains(Actor->GetOwner())) return false;
    }
    if(Object->HasAnyFlags(RF_WasLoaded)) return false;
    return Object->IsInOuter(CurrentObject);
}

FArchive& FAAObjectCollectorArchive::operator<<(UObject*& Value)
{
    if(Value && Value->Implements<UFGSaveInterface>() && ShouldSave(Value))
        this->AddObject(Value);
    return *this;
}

FArchive& FAAObjectCollectorArchive::operator<<(FLazyObjectPtr& Value)
{
    if (Value.IsValid() && Value.Get() && Value.Get()->Implements<UFGSaveInterface>() && ShouldSave(Value.Get()))
        this->AddObject(Value.Get());
    return *this;
}

FArchive& FAAObjectCollectorArchive::operator<<(FSoftObjectPtr& Value)
{
    if (Value.IsValid() && Value.Get() && Value.Get()->Implements<UFGSaveInterface>() && ShouldSave(Value.Get()))
        this->AddObject(Value.Get());
    return *this;
}

FArchive& FAAObjectCollectorArchive::operator<<(FSoftObjectPath& Value)
{
    return *this;
}

FArchive& FAAObjectCollectorArchive::operator<<(FWeakObjectPtr& Value)
{
    if (Value.IsValid() && Value.Get() && Value.Get()->Implements<UFGSaveInterface>() && ShouldSave(Value.Get()))
        this->AddObject(Value.Get());
    return *this;
}

