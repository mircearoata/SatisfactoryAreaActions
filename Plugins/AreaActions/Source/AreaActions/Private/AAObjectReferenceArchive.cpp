// Fill out your copyright notice in the Description page of Project Settings.

#include "AAObjectReferenceArchive.h"

#include "SaveCustomVersion.h"

FAAObjectReferenceArchive::FAAObjectReferenceArchive(FArchive& InnerArchive, TArray<UObject*> InObjects): FObjectAndNameAsStringProxyArchive(
    InnerArchive, false)
{
    ArIsSaveGame = true;
    this->Objects = InObjects;
    UsingCustomVersion(FSaveCustomVersion::GUID);
}

FArchive& FAAObjectReferenceArchive::operator<<(UObject*& Obj)
{
    if (IsLoading())
    {
        // load the path name to the object
        int32 Idx;
        FString RelativePath;
        InnerArchive << Idx;
        InnerArchive << RelativePath;
        Obj = ResolveReference(Idx, RelativePath);
    }
    else
    {
        int32 Idx = -1;
        FString RelativePath;
        GetReferenceData(Obj, Idx, RelativePath);
        InnerArchive << Idx;
        InnerArchive << RelativePath;
    }
    return *this;
}

void FAAObjectReferenceArchive::GetReferenceData(UObject* Obj, int32& Idx, FString& RelativePath) const
{
    Idx = -1;
    UObject* Outer = nullptr;
    if (Obj)
    {
        Outer = Obj;
        while (Outer && !Objects.Contains(Outer))
            Outer = Outer->GetOuter();
        Idx = Objects.Find(Outer);
    }
    RelativePath = Obj->GetPathName(Outer);
}

UObject* FAAObjectReferenceArchive::ResolveReference(const int32& Idx, const FString& RelativePath)
{
    UObject* Obj;
    if (Idx == -1)
    {
        // is not copied, reference the same object
        Obj = FindObject<UObject>(nullptr, *RelativePath, false);
    }
    else if (RelativePath == TEXT("None"))
    {
        // It is one of the copied objects
        Obj = Objects[Idx];
    }
    else
    {
        // look up the object by fully qualified pathname
        Obj = FindObject<UObject>(Objects[Idx], *RelativePath, false);
    }
    return Obj;
}
