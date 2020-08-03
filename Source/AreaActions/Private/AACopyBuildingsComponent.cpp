// Fill out your copyright notice in the Description page of Project Settings.

#include "AACopyBuildingsComponent.h"

#include "SML/util/Logging.h"
#include "FGColoredInstanceMeshProxy.h"
#include "FGFactorySettings.h"
#include "GameFramework/Actor.h"

FVector FRotatedBoundingBox::GetCorner(const uint32 CornerNum) const
{
    switch(CornerNum)
    {
    case 0:
        return Center + Rotation.RotateVector(FVector(Extents.X, Extents.Y, 0));
    case 1:
        return Center + Rotation.RotateVector(FVector(Extents.X, -Extents.Y, 0));
    case 2:
        return Center + Rotation.RotateVector(FVector(-Extents.X, -Extents.Y, 0));
    case 3:
        return Center + Rotation.RotateVector(FVector(-Extents.X, Extents.Y, 0));
    default:
        return Center;
    }
}

UAACopyBuildingsComponent::UAACopyBuildingsComponent()
{
    ValidCheckSkipProperties.Add(AActor::StaticClass()->FindPropertyByName(TEXT("Owner")));
}

bool UAACopyBuildingsComponent::SetActors(TArray<AActor*>& Actors, TArray<AFGBuildable*>& OutBuildingsWithIssues)
{
    TArray<AFGBuildable*> Buildings;
    for (AActor* Actor : Actors)
        if (Actor->IsA<AFGBuildable>())
            Buildings.Add(static_cast<AFGBuildable*>(Actor));
    return SetBuildings(Buildings, OutBuildingsWithIssues);
}

bool UAACopyBuildingsComponent::SetBuildings(TArray<AFGBuildable*>& Buildings,
                                             TArray<AFGBuildable*>& OutBuildingsWithIssues)
{
    this->OriginalBuildings = Buildings;
    Algo::Sort(this->OriginalBuildings, [](AFGBuildable* A, AFGBuildable* B)
    {
        return A->GetBuildTime() < B->GetBuildTime();
    });
    const bool Ret = ValidateBuildings(OutBuildingsWithIssues);
    if(Ret)
        CalculateBounds();
    return Ret;
}

bool UAACopyBuildingsComponent::ValidateObject(UObject* Object)
{
    for (TFieldIterator<UObjectPropertyBase> PropertyIterator(Object->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        UObjectPropertyBase* Property = *PropertyIterator;
        bool bPropertyValid = true;
        if (Property->HasAllPropertyFlags(CPF_SaveGame))
        {
            for (int i = 0; i < Property->ArrayDim; i++)
            {
                void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Object, i);
                UObject* Value = Property->GetObjectPropertyValue(ValuePtr);

                if (!Value) continue;

                FString Name = Value->GetPathName();
                if (!Name.StartsWith(GetWorld()->GetPathName())) continue;

                bool bPropertyElementValid = false;
                for (AFGBuildable* OtherBuilding : this->OriginalBuildings)
                {
                    if (Name.Contains(*(OtherBuilding->GetPathName() + ".")) || Value == OtherBuilding)
                    {
                        bPropertyElementValid = true;
                        break;
                    }
                }

                if (!bPropertyElementValid)
                {
                    SML::Logging::error(*Property->GetPathName(), "[", i, "]=", *Name);
                    bPropertyValid = false;
                    break;
                }
            }
        }
        if (!bPropertyValid)
            return false;
    }

    for (TFieldIterator<UArrayProperty> PropertyIterator(Object->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        UArrayProperty* Property = *PropertyIterator;
        bool bPropertyValid = true;
        if (Property->HasAllPropertyFlags(CPF_SaveGame))
        {
            UProperty* InnerProperty = Property->Inner;
            if (InnerProperty->IsA<UObjectPropertyBase>())
            {
                UObjectPropertyBase* CastedInnerProperty = Cast<UObjectPropertyBase>(InnerProperty);
                void* Arr = Property->ContainerPtrToValuePtr<void>(Object);
                FScriptArrayHelper ArrayHelper(Property, Arr);

                for (int i = 0; i < ArrayHelper.Num(); i++)
                {
                    UObject* Value = CastedInnerProperty->GetObjectPropertyValue(ArrayHelper.GetRawPtr(i));
                    if (!Value) continue;

                    FString Name = Value->GetPathName();
                    if (!Name.StartsWith(GetWorld()->GetPathName())) continue;

                    bool bPropertyElementValid = false;
                    for (AFGBuildable* OtherBuilding : this->OriginalBuildings)
                    {
                        if (Name.Contains(*(OtherBuilding->GetPathName() + ".")) || Value == OtherBuilding)
                        {
                            bPropertyElementValid = true;
                            break;
                        }
                    }

                    if (!bPropertyElementValid)
                    {
                        SML::Logging::error(*Property->GetPathName(), "[", i, "]=", *Name);
                        bPropertyValid = false;
                        break;
                    }
                }
            }
        }
        if (!bPropertyValid)
            return false;
    }

    for (TFieldIterator<UMapProperty> PropertyIterator(Object->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        UMapProperty* Property = *PropertyIterator;
        bool bPropertyValid = true;
        if (Property->HasAllPropertyFlags(CPF_SaveGame))
        {
            UProperty* KeyProp = Property->KeyProp;
            UProperty* ValProp = Property->ValueProp;
            bool bIsKeyObject = KeyProp->IsA<UObjectPropertyBase>();
            bool bIsValObject = ValProp->IsA<UObjectPropertyBase>();
            if (bIsKeyObject || bIsValObject)
            {
                UObjectPropertyBase* CastedInnerProperty = Cast<UObjectPropertyBase>(KeyProp);
                void* Map = Property->ContainerPtrToValuePtr<void>(Object);
                FScriptMapHelper MapHelper(Property, Map);
                for (int i = 0; i < MapHelper.Num(); i++)
                {
                    if (bIsKeyObject)
                    {
                        UObject* Value = CastedInnerProperty->GetObjectPropertyValue(MapHelper.GetKeyPtr(i));
                        if (!Value) continue;

                        FString Name = Value->GetPathName();
                        if (!Name.StartsWith(GetWorld()->GetPathName())) continue;

                        bool bPropertyElementValid = false;
                        for (AFGBuildable* OtherBuilding : this->OriginalBuildings)
                        {
                            if (Name.Contains(*(OtherBuilding->GetPathName() + ".")) || Value == OtherBuilding)
                            {
                                bPropertyElementValid = true;
                                break;
                            }
                        }

                        if (!bPropertyElementValid)
                        {
                            SML::Logging::error(*Property->GetPathName(), "[", i, "]=", *Name);
                            bPropertyValid = false;
                            break;
                        }
                    }
                    if (bIsValObject)
                    {
                        UObject* Value = CastedInnerProperty->GetObjectPropertyValue(MapHelper.GetValuePtr(i));
                        if (!Value) continue;

                        FString Name = Value->GetPathName();
                        if (!Name.StartsWith(GetWorld()->GetPathName())) continue;

                        bool bPropertyElementValid = false;
                        for (AFGBuildable* OtherBuilding : this->OriginalBuildings)
                        {
                            if (Name.Contains(*(OtherBuilding->GetPathName() + ".")) || Value == OtherBuilding)
                            {
                                bPropertyElementValid = true;
                                break;
                            }
                        }

                        if (!bPropertyElementValid)
                        {
                            SML::Logging::error(*Property->GetPathName(), "[", i, "]=", *Name);
                            bPropertyValid = false;
                            break;
                        }
                    }
                }
            }
        }
        if (!bPropertyValid)
            return false;
    }

    for (TFieldIterator<USetProperty> PropertyIterator(Object->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        USetProperty* Property = *PropertyIterator;
        bool bPropertyValid = true;
        if (Property->HasAllPropertyFlags(CPF_SaveGame))
        {
            UProperty* KeyProp = Property->ElementProp;
            if (KeyProp->IsA<UObjectPropertyBase>())
            {
                UObjectPropertyBase* CastedInnerProperty = Cast<UObjectPropertyBase>(KeyProp);
                void* Set = Property->ContainerPtrToValuePtr<void>(Object);
                FScriptSetHelper SetHelper(Property, Set);
                for (int i = 0; i < SetHelper.Num(); i++)
                {
                    UObject* Value = CastedInnerProperty->GetObjectPropertyValue(SetHelper.GetElementPtr(i));
                    if (!Value) continue;

                    FString Name = Value->GetPathName();
                    if (!Name.StartsWith(GetWorld()->GetPathName())) continue;

                    bool bPropertyElementValid = false;
                    for (AFGBuildable* OtherBuilding : this->OriginalBuildings)
                    {
                        if (Name.Contains(*(OtherBuilding->GetPathName() + ".")) || Value == OtherBuilding)
                        {
                            bPropertyElementValid = true;
                            break;
                        }
                    }

                    if (!bPropertyElementValid)
                    {
                        SML::Logging::error(*Property->GetPathName(), "[", i, "]=", *Name);
                        bPropertyValid = false;
                        break;
                    }
                }
            }
        }
        if (!bPropertyValid)
            return false;
    }
    return true;
}

void UAACopyBuildingsComponent::CalculateBounds()
{
    TMap<float, uint32> RotationCount;
    for(AActor* Actor : this->OriginalBuildings)
        RotationCount.FindOrAdd(FGenericPlatformMath::Fmod(FGenericPlatformMath::Fmod(Actor->GetActorRotation().Yaw, 90) + 90, 90))++;

    RotationCount.ValueSort([](const uint32& A, const uint32& B) {
        return A > B;
    });

    const FRotator Rotation = FRotator(0, (*RotationCount.CreateIterator()).Key, 0);

    FVector Min = FVector(TNumericLimits<float>::Max());
    FVector Max = FVector(-TNumericLimits<float>::Max());
    
    for(AFGBuildable* Buildable : this->OriginalBuildings)
    {
        if(UShapeComponent* Clearance = Buildable->GetClearanceComponent())
        {
            if(UBoxComponent* Box = Cast<UBoxComponent>(Clearance))
            {
                const FVector Extents = Box->GetScaledBoxExtent();
                for(int i = 0; i < (1 << 3); i++)
                {
                    const int X = (i & 1) ? 1 : -1;
                    const int Y = (i & 2) ? 1 : -1;
                    const int Z = (i & 4) ? 1 : -1;
                    FVector Corner = FVector(Extents.X * X, Extents.Y * Y, Extents.Z * Z);
                    Min = Min.ComponentMin(Rotation.UnrotateVector(Buildable->GetActorRotation().RotateVector(Box->GetComponentTransform().GetLocation() + Corner - Buildable->GetActorLocation()) + Buildable->GetActorLocation()));
                    Max = Max.ComponentMax(Rotation.UnrotateVector(Buildable->GetActorRotation().RotateVector(Box->GetComponentTransform().GetLocation() + Corner - Buildable->GetActorLocation()) + Buildable->GetActorLocation()));
                }
            }
            else
            {
                // Are there any other types used as clearance?
            }
        }
        else
        {
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            Params.bDeferConstruction = true;
            AFGBuildable* PreviewBuilding = this->GetWorld()->SpawnActor<AFGBuildable>(
                Buildable->GetClass(), FTransform::Identity, Params);
            PreviewBuilding->bDeferBeginPlay = true;
            PreviewBuilding->FinishSpawning(FTransform::Identity, true);
            FVector Origin;
            FVector Extents;
            PreviewBuilding->GetActorBounds(true, Origin, Extents);
            Extents = FVector(FGenericPlatformMath::RoundToFloat(Extents.X), FGenericPlatformMath::RoundToFloat(Extents.Y), FGenericPlatformMath::RoundToFloat(Extents.Z));

            for(int i = 0; i < (1 << 3); i++)
            {
                const int X = (i & 1) ? 1 : -1;
                const int Y = (i & 2) ? 1 : -1;
                const int Z = (i & 4) ? 1 : -1;
                FVector Corner = FVector(Extents.X * X, Extents.Y * Y, Extents.Z * Z);
                Min = Min.ComponentMin(Rotation.UnrotateVector(Buildable->GetActorLocation() + Buildable->GetActorRotation().RotateVector(Origin + Corner)));
                Max = Max.ComponentMax(Rotation.UnrotateVector(Buildable->GetActorLocation() + Buildable->GetActorRotation().RotateVector(Origin + Corner)));
            }
            PreviewBuilding->Destroy();
        }
    }

    Min = Rotation.RotateVector(Min);
    Max = Rotation.RotateVector(Max);

    const FVector Center = (Min + Max) / 2;

    const FVector Bounds = Rotation.UnrotateVector(Max - Center);
            
    this->BuildingsBounds = FRotatedBoundingBox{Center, FVector(FGenericPlatformMath::RoundToFloat(Bounds.X), FGenericPlatformMath::RoundToFloat(Bounds.Y), FGenericPlatformMath::RoundToFloat(Bounds.Z)), Rotation};
}

bool UAACopyBuildingsComponent::ValidateBuildings(TArray<AFGBuildable*>& OutBuildingsWithIssues)
{
    for (AFGBuildable* Buildable : this->OriginalBuildings)
    {
        if (!ValidateObject(Buildable))
        {
            OutBuildingsWithIssues.Add(Buildable);
            continue;
        }
        bool bComponentsValid = true;
        for (UActorComponent* Component : Buildable->GetComponents())
        {
            if (!ValidateObject(Component))
            {
                bComponentsValid = false;
                break;
            }
        }
        if (!bComponentsValid)
        {
            OutBuildingsWithIssues.Add(Buildable);
        }
    }
    return OutBuildingsWithIssues.Num() == 0;
}

void CopyBySerialization(UObject* From, UObject* To)
{
    TArray<uint8> SerializedBytes;
    FMemoryWriter ActorWriter = FMemoryWriter(SerializedBytes, true);
    FObjectAndNameAsStringProxyArchive Ar(ActorWriter, true);
    Ar.SetIsLoading(false);
    Ar.SetIsSaving(true);
    Ar.ArIsSaveGame = true;
    From->Serialize(Ar);

    FMemoryReader ActorReader = FMemoryReader(SerializedBytes, true);
    FObjectAndNameAsStringProxyArchive Ar2(ActorReader, true);
    Ar2.SetIsLoading(true);
    Ar2.SetIsSaving(false);
    Ar2.ArIsSaveGame = true;
    To->Serialize(Ar2);
}

template <typename T>
T* GetObjectReplaceReference(T* Obj, T* From, T* To)
{
    if (Obj == From)
        return To;

    FString NewName = Obj->GetPathName();
    if (!NewName.Contains(*(From->GetPathName() + "."))) return nullptr;

    NewName = NewName.Replace(*(From->GetPathName() + "."), *(To->GetPathName() + "."));
    return FindObject<T>(nullptr, *NewName);
}

void FixReferencesToBuilding(UObject* From, UObject* To, UObject* ReferenceFrom, UObject* ReferenceTo)
{
    for (TFieldIterator<UObjectPropertyBase> PropertyIterator(From->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        UObjectPropertyBase* Property = *PropertyIterator;
        if (Property->HasAllPropertyFlags(CPF_SaveGame))
        {
            for (int i = 0; i < Property->ArrayDim; i++)
            {
                void* OriginalValuePtr = Property->ContainerPtrToValuePtr<void>(From, i);
                void* PreviewValuePtr = Property->ContainerPtrToValuePtr<void>(To, i);
                UObject* Original = Property->GetObjectPropertyValue(OriginalValuePtr);

                if (!Original) continue;

                FString OriginalName = Original->GetPathName();
                FString NewName = Original->GetPathName();
                if (!OriginalName.Contains(*(ReferenceFrom->GetPathName() + ".")) && OriginalName != ReferenceFrom->
                    GetPathName()) continue;

                UObject* NewObject = GetObjectReplaceReference<UObject>(Original, ReferenceFrom, ReferenceTo);
                Property->SetObjectPropertyValue(PreviewValuePtr, NewObject);
            }
        }
    }

    for (TFieldIterator<UArrayProperty> PropertyIterator(From->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        UArrayProperty* Property = *PropertyIterator;
        if (Property->HasAllPropertyFlags(CPF_SaveGame))
        {
            UProperty* InnerProperty = Property->Inner;
            if (InnerProperty->IsA<UObjectPropertyBase>())
            {
                UObjectPropertyBase* CastedInnerProperty = Cast<UObjectPropertyBase>(InnerProperty);
                void* OriginalArr = Property->ContainerPtrToValuePtr<void>(From);
                void* PreviewArr = Property->ContainerPtrToValuePtr<void>(To);
                FScriptArrayHelper OriginalArrayHelper(Property, OriginalArr);
                FScriptArrayHelper PreviewArrayHelper(Property, PreviewArr);
                PreviewArrayHelper.Resize(OriginalArrayHelper.Num());

                for (int i = 0; i < OriginalArrayHelper.Num(); i++)
                {
                    UObject* Original = CastedInnerProperty->GetObjectPropertyValue(OriginalArrayHelper.GetRawPtr(i));
                    if (!Original) continue;

                    FString OriginalName = Original->GetPathName();
                    FString NewName = Original->GetPathName();
                    if (!OriginalName.Contains(*(ReferenceFrom->GetPathName() + ".")) && OriginalName != ReferenceFrom->
                        GetPathName()) continue;

                    UObject* NewObject = GetObjectReplaceReference<UObject>(Original, ReferenceFrom, ReferenceTo);
                    CastedInnerProperty->SetObjectPropertyValue(PreviewArrayHelper.GetRawPtr(i), NewObject);
                }
            }
        }
    }

    for (TFieldIterator<UMapProperty> PropertyIterator(From->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        UMapProperty* Property = *PropertyIterator;
        if (Property->HasAllPropertyFlags(CPF_SaveGame))
        {
            UProperty* KeyProp = Property->KeyProp;
            UProperty* ValProp = Property->ValueProp;
            bool bIsKeyObject = KeyProp->IsA<UObjectPropertyBase>();
            bool bIsValObject = ValProp->IsA<UObjectPropertyBase>();
            if (bIsKeyObject || bIsValObject)
            {
                UObjectPropertyBase* CastedInnerProperty = Cast<UObjectPropertyBase>(KeyProp);
                void* OriginalMap = Property->ContainerPtrToValuePtr<void>(From);
                void* PreviewMap = Property->ContainerPtrToValuePtr<void>(To);
                FScriptMapHelper OriginalMapHelper(Property, OriginalMap);
                FScriptMapHelper PreviewMapHelper(Property, PreviewMap);
                for (int i = 0; i < OriginalMapHelper.Num(); i++)
                    PreviewMapHelper.AddPair(OriginalMapHelper.GetKeyPtr(i), OriginalMapHelper.GetValuePtr(i)); // Is this right?
                for (int i = 0; i < OriginalMapHelper.Num(); i++)
                {
                    if (bIsKeyObject)
                    {
                        UObject* Original = CastedInnerProperty->GetObjectPropertyValue(OriginalMapHelper.GetKeyPtr(i));
                        if (!Original) continue;

                        FString OriginalName = Original->GetPathName();
                        FString NewName = Original->GetPathName();
                        if (!OriginalName.Contains(*(ReferenceFrom->GetPathName() + ".")) && OriginalName !=
                            ReferenceFrom->GetPathName()) continue;

                        UObject* NewObject = GetObjectReplaceReference<UObject>(Original, ReferenceFrom, ReferenceTo);
                        CastedInnerProperty->SetObjectPropertyValue(PreviewMapHelper.GetKeyPtr(i), NewObject);
                    }
                    if (bIsValObject)
                    {
                        UObject* Original = CastedInnerProperty->GetObjectPropertyValue(
                            OriginalMapHelper.GetValuePtr(i));
                        if (!Original) continue;

                        FString OriginalName = Original->GetPathName();
                        FString NewName = Original->GetPathName();
                        if (!OriginalName.Contains(*(ReferenceFrom->GetPathName() + ".")) && OriginalName !=
                            ReferenceFrom->GetPathName()) continue;

                        UObject* NewObject = GetObjectReplaceReference<UObject>(Original, ReferenceFrom, ReferenceTo);
                        CastedInnerProperty->SetObjectPropertyValue(PreviewMapHelper.GetValuePtr(i), NewObject);
                    }
                }
                PreviewMapHelper.Rehash();
            }
        }
    }

    for (TFieldIterator<USetProperty> PropertyIterator(From->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        USetProperty* Property = *PropertyIterator;
        if (Property->HasAllPropertyFlags(CPF_SaveGame))
        {
            UProperty* KeyProp = Property->ElementProp;
            if (KeyProp->IsA<UObjectPropertyBase>())
            {
                UObjectPropertyBase* CastedInnerProperty = Cast<UObjectPropertyBase>(KeyProp);
                void* OriginalSet = Property->ContainerPtrToValuePtr<void>(From);
                void* PreviewSet = Property->ContainerPtrToValuePtr<void>(To);
                FScriptSetHelper OriginalSetHelper(Property, OriginalSet);
                FScriptSetHelper PreviewSetHelper(Property, PreviewSet);
                for (int i = 0; i < OriginalSetHelper.Num(); i++)
                    PreviewSetHelper.AddUninitializedValue();
                for (int i = 0; i < OriginalSetHelper.Num(); i++)
                {
                    UObject* Original = CastedInnerProperty->GetObjectPropertyValue(OriginalSetHelper.GetElementPtr(i));
                    if (!Original) continue;

                    FString OriginalName = Original->GetPathName();
                    FString NewName = Original->GetPathName();
                    if (!OriginalName.Contains(*(ReferenceFrom->GetPathName() + "."))
                        && OriginalName != ReferenceFrom->GetPathName()) continue;

                    UObject* NewObject = GetObjectReplaceReference<UObject>(Original, ReferenceFrom, ReferenceTo);
                    CastedInnerProperty->SetObjectPropertyValue(PreviewSetHelper.GetElementPtr(i), NewObject);
                }
                PreviewSetHelper.Rehash();
            }
        }
    }
}

void PreSaveGame(UObject* Object)
{
    if (Object->Implements<UFGSaveInterface>())
        IFGSaveInterface::Execute_PreSaveGame(
            Object, UFGSaveSession::GetVersion(UFGSaveSession::Get(Object->GetWorld())->mSaveHeader),
            FEngineVersion::Current().GetChangelist());
}

void PostSaveGame(UObject* Object)
{
    if (Object->Implements<UFGSaveInterface>())
        IFGSaveInterface::Execute_PostSaveGame(
            Object, UFGSaveSession::GetVersion(UFGSaveSession::Get(Object->GetWorld())->mSaveHeader),
            FEngineVersion::Current().GetChangelist());
}

void PreLoadGame(UObject* Object)
{
    if (Object->Implements<UFGSaveInterface>())
        IFGSaveInterface::Execute_PreLoadGame(
            Object, UFGSaveSession::GetVersion(UFGSaveSession::Get(Object->GetWorld())->mSaveHeader),
            FEngineVersion::Current().GetChangelist());
}

void PostLoadGame(UObject* Object)
{
    if (Object->Implements<UFGSaveInterface>())
        IFGSaveInterface::Execute_PostLoadGame(
            Object, UFGSaveSession::GetVersion(UFGSaveSession::Get(Object->GetWorld())->mSaveHeader),
            FEngineVersion::Current().GetChangelist());
}

FTransform TransformAroundPoint(const FTransform Original, const FVector Offset, const FRotator Rotation, const FVector RotationCenter)
{
    const FVector NewLocation = Rotation.RotateVector(Original.GetLocation() - RotationCenter) + RotationCenter + Offset;
    const FRotator NewRotation = Original.Rotator() + Rotation;
    return FTransform(NewRotation, NewLocation, Original.GetScale3D());
}

int UAACopyBuildingsComponent::AddCopy(const FVector Offset, const FRotator Rotation, const FVector RotationCenter, const bool Relative)
{
    for (AFGBuildable* Buildable : this->OriginalBuildings)
    {
        FTransform NewTransform = TransformAroundPoint(Buildable->GetActorTransform(), Relative ? BuildingsBounds.Rotation.RotateVector(Offset) : Offset, Rotation, RotationCenter);
        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        Params.bDeferConstruction = true;
        AFGBuildable* PreviewBuilding = this->GetWorld()->SpawnActor<AFGBuildable>(
            Buildable->GetClass(), NewTransform, Params);
        PreviewBuilding->bDeferBeginPlay = true;
        PreviewBuilding->FinishSpawning(FTransform::Identity, true);
        this->BuildingsPreview.FindOrAdd(Buildable).Buildings.Add(CurrentId, PreviewBuilding);

        for (UActorComponent* NewComponent : PreviewBuilding->GetComponents())
            PreLoadGame(NewComponent);
        PreLoadGame(PreviewBuilding);

        for (UActorComponent* Component : Buildable->GetComponents())
            PreSaveGame(Component);
        PreSaveGame(Buildable);
    }
    for (int i = 0; i < this->OriginalBuildings.Num(); i++)
    {
        AFGBuildable* Buildable = this->OriginalBuildings[i];
        AFGBuildable* PreviewBuilding = this->BuildingsPreview.Find(Buildable)->Buildings[CurrentId];

        CopyBySerialization(Buildable, PreviewBuilding);

        for (UActorComponent* Component : Buildable->GetComponents())
        {
            if (!Component->Implements<UFGSaveInterface>()) continue;
            UActorComponent* PreviewComponent = nullptr;
            for (UActorComponent* NewComponent : PreviewBuilding->GetComponents())
                if (NewComponent->GetName() == Component->GetName())
                {
                    PreviewComponent = NewComponent;
                    break;
                }

            if (!PreviewComponent)
            {
                PreviewComponent = NewObject<UActorComponent>(
                    GetObjectReplaceReference<UObject>(Component->GetOuter(), Buildable, PreviewBuilding),
                    Component->GetClass(), FName(*Component->GetName()), Component->GetFlags());
                PreviewComponent->RegisterComponent();
                if (PreviewComponent->IsA<USceneComponent>())
                    SML::Logging::fatal(*FString::Printf(
                        TEXT("Component %s of %s is a scene component. Not implemented yet!"), *Component->GetName(),
                        *Buildable->GetName())); // will this ever happen?
                // TODO: SceneComponents?
            }

            CopyBySerialization(Component, PreviewComponent);
        }
    }

    for (int i = 0; i < this->OriginalBuildings.Num(); i++)
    {
        AFGBuildable* Buildable = this->OriginalBuildings[i];
        AFGBuildable* PreviewBuilding = this->BuildingsPreview.Find(Buildable)->Buildings[CurrentId];
        for (int j = 0; j < this->OriginalBuildings.Num(); j++)
        {
            AFGBuildable* OtherBuildable = this->OriginalBuildings[j];
            AFGBuildable* OtherPreviewBuilding = this->BuildingsPreview.Find(OtherBuildable)->Buildings[CurrentId];
            FixReferencesToBuilding(OtherBuildable, OtherPreviewBuilding, Buildable, PreviewBuilding);

            for (UActorComponent* Component : OtherBuildable->GetComponents())
            {
                if (!Component->Implements<UFGSaveInterface>()) continue;
                UActorComponent* PreviewComponent = nullptr;
                for (UActorComponent* NewComponent : OtherPreviewBuilding->GetComponents())
                {
                    if (NewComponent->GetName() == Component->GetName())
                    {
                        PreviewComponent = NewComponent;
                        break;
                    }
                }

                if (!PreviewComponent)
                {
                    SML::Logging::fatal(*FString::Printf(
                        TEXT("Component %s of %s does not exist. This shouldn't happen!"), *Component->GetName(),
                        *PreviewBuilding->GetName())); // I would like SML::Logging::wtf
                    continue;
                }

                FixReferencesToBuilding(Component, PreviewComponent, Buildable, PreviewBuilding);
            }
        }
    }

    for (int i = 0; i < this->OriginalBuildings.Num(); i++)
    {
        AFGBuildable* Buildable = this->OriginalBuildings[i];
        AFGBuildable* PreviewBuilding = this->BuildingsPreview.Find(Buildable)->Buildings[CurrentId];
        for (UActorComponent* NewComponent : PreviewBuilding->GetComponents())
            PostLoadGame(NewComponent);
        PostLoadGame(PreviewBuilding);

        for (UActorComponent* Component : Buildable->GetComponents())
            PostSaveGame(Component);
        PostSaveGame(Buildable);
    }

    for (int i = 0; i < this->OriginalBuildings.Num(); i++)
    {
        AFGBuildable* Buildable = this->OriginalBuildings[i];
        AFGBuildable* PreviewBuilding = this->BuildingsPreview.Find(Buildable)->Buildings[CurrentId];
        PreviewBuilding->DeferredBeginPlay();
        TArray<UFGColoredInstanceMeshProxy*> ColoredInstanceMeshProxies;
        PreviewBuilding->GetComponents<UFGColoredInstanceMeshProxy>(ColoredInstanceMeshProxies);
        for (UFGColoredInstanceMeshProxy* Mesh : ColoredInstanceMeshProxies)
            Mesh->SetInstanced(false);
        if(ColoredInstanceMeshProxies.Num() > 0)
        {
            UFGColoredInstanceMeshProxy* Mesh = ColoredInstanceMeshProxies[0];
            for (int MatNum = 0; MatNum < Mesh->GetNumMaterials(); MatNum++)
                Mesh->SetMaterial(MatNum, UFGFactorySettings::Get()->mDefaultValidPlacementMaterial);
        }
    }
    return CurrentId++;
}

void UAACopyBuildingsComponent::MoveCopy(const int CopyId, const FVector Offset, const FRotator Rotation, const FVector RotationCenter, const bool Relative)
{
    for (AFGBuildable* Buildable : this->OriginalBuildings)
    {
        FTransform NewTransform = TransformAroundPoint(Buildable->GetActorTransform(), Relative ? BuildingsBounds.Rotation.RotateVector(Offset) : Offset, Rotation, RotationCenter);
        AFGBuildable* PreviewBuilding = this->BuildingsPreview[Buildable].Buildings[CopyId];
        const EComponentMobility::Type CurrentMobility = PreviewBuilding->GetRootComponent()->Mobility;
        PreviewBuilding->GetRootComponent()->SetMobility(EComponentMobility::Movable);
        PreviewBuilding->SetActorTransform(NewTransform);
        PreviewBuilding->GetRootComponent()->SetMobility(CurrentMobility);
    }
}

void UAACopyBuildingsComponent::RemoveCopy(const int CopyId)
{
    for (AFGBuildable* Buildable : this->OriginalBuildings)
    {
        AFGBuildable* PreviewBuilding = this->BuildingsPreview[Buildable].Buildings[CopyId];
        this->BuildingsPreview[Buildable].Buildings.Remove(CopyId);
        PreviewBuilding->Destroy();
    }
}

void UAACopyBuildingsComponent::Finish()
{
    TArray<AFGBuildable*> PreviewBuildingKeys;
    this->BuildingsPreview.GetKeys(PreviewBuildingKeys);
    for (AFGBuildable* Buildable : PreviewBuildingKeys)
    {
        for (const auto Preview : this->BuildingsPreview[Buildable].Buildings)
        {
            AFGBuildable* PreviewBuilding = Preview.Value;
            TArray<UFGColoredInstanceMeshProxy*> ColoredInstanceMeshProxies;
            PreviewBuilding->GetComponents<UFGColoredInstanceMeshProxy>(ColoredInstanceMeshProxies);
            for (UFGColoredInstanceMeshProxy* Mesh : ColoredInstanceMeshProxies)
            {
                Mesh->SetInstanced(true);
                UFGColoredInstanceMeshProxy* OriginalMeshComponent = nullptr;
                TArray<UFGColoredInstanceMeshProxy*> OriginalColoredInstanceMeshProxies;
                PreviewBuilding->GetComponents<UFGColoredInstanceMeshProxy>(OriginalColoredInstanceMeshProxies);
                for (UFGColoredInstanceMeshProxy* OriginalBuildingMesh : OriginalColoredInstanceMeshProxies)
                {
                    if (OriginalBuildingMesh->GetName() == Mesh->GetName())
                    {
                        OriginalMeshComponent = OriginalBuildingMesh;
                        break;
                    }
                }

                if (!OriginalMeshComponent)
                {
                    SML::Logging::fatal(*FString::Printf(
                        TEXT("Mesh component %s of %s does not exist. This shouldn't happen!"),
                        *Mesh->GetName(), *Buildable->GetName())); // I would like SML::Logging::wtf
                    continue;
                }
                
                for (int MatNum = 0; MatNum < Mesh->GetNumMaterials(); MatNum++)
                    Mesh->SetMaterial(MatNum, OriginalMeshComponent->GetMaterial(MatNum));
            }
        }
    }
}