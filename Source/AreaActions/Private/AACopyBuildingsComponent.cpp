// Fill out your copyright notice in the Description page of Project Settings.

#include "AACopyBuildingsComponent.h"

#include "SML/util/Logging.h"
#include "FGColoredInstanceMeshProxy.h"
#include "FGFactorySettings.h"
#include "GameFramework/Actor.h"
#pragma optimize( "", off )

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

bool UAACopyBuildingsComponent::SetActors(TArray<AActor*>& actors, TArray<AFGBuildable*>& OutBuildingsWithIssues)
{
    TArray<AFGBuildable*> buildings;
    for (AActor* actor : actors)
        if (actor->IsA<AFGBuildable>())
            buildings.Add(static_cast<AFGBuildable*>(actor));
    return SetBuildings(buildings, OutBuildingsWithIssues);
}

bool UAACopyBuildingsComponent::SetBuildings(TArray<AFGBuildable*>& buildings,
                                             TArray<AFGBuildable*>& OutBuildingsWithIssues)
{
    this->OriginalBuildings = buildings;
    Algo::Sort(this->OriginalBuildings, [](AFGBuildable* a, AFGBuildable* b)
    {
        return a->GetBuildTime() < b->GetBuildTime();
    });
    const bool Ret = ValidateBuildings(OutBuildingsWithIssues);
    if(Ret)
        CalculateBounds();
    return Ret;
}

bool UAACopyBuildingsComponent::ValidateObject(UObject* obj)
{
    for (TFieldIterator<UObjectPropertyBase> PropertyIterator(obj->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        UObjectPropertyBase* property = *PropertyIterator;
        bool propertyValid = true;
        if (property->HasAllPropertyFlags(CPF_SaveGame))
        {
            for (int i = 0; i < property->ArrayDim; i++)
            {
                void* valuePtr = property->ContainerPtrToValuePtr<void>(obj, i);
                UObject* value = property->GetObjectPropertyValue(valuePtr);

                if (!value) continue;

                FString name = value->GetPathName();
                if (!name.StartsWith(GetWorld()->GetPathName())) continue;

                bool propertyElementValid = false;
                for (AFGBuildable* otherBuilding : this->OriginalBuildings)
                {
                    if (name.Contains(*(otherBuilding->GetPathName() + ".")) || value == otherBuilding)
                    {
                        propertyElementValid = true;
                        break;
                    }
                }

                if (!propertyElementValid)
                {
                    SML::Logging::error(*property->GetPathName(), "[", i, "]=", *name);
                    propertyValid = false;
                    break;
                }
            }
        }
        if (!propertyValid)
            return false;
    }

    for (TFieldIterator<UArrayProperty> PropertyIterator(obj->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        UArrayProperty* property = *PropertyIterator;
        bool propertyValid = true;
        if (property->HasAllPropertyFlags(CPF_SaveGame))
        {
            UProperty* innerProperty = property->Inner;
            if (innerProperty->IsA<UObjectPropertyBase>())
            {
                UObjectPropertyBase* castedInnerProperty = Cast<UObjectPropertyBase>(innerProperty);
                void* arr = property->ContainerPtrToValuePtr<void>(obj);
                FScriptArrayHelper ArrayHelper(property, arr);

                for (int i = 0; i < ArrayHelper.Num(); i++)
                {
                    UObject* value = castedInnerProperty->GetObjectPropertyValue(ArrayHelper.GetRawPtr(i));
                    if (!value) continue;

                    FString name = value->GetPathName();
                    if (!name.StartsWith(GetWorld()->GetPathName())) continue;

                    bool propertyElementValid = false;
                    for (AFGBuildable* otherBuilding : this->OriginalBuildings)
                    {
                        if (name.Contains(*(otherBuilding->GetPathName() + ".")) || value == otherBuilding)
                        {
                            propertyElementValid = true;
                            break;
                        }
                    }

                    if (!propertyElementValid)
                    {
                        SML::Logging::error(*property->GetPathName(), "[", i, "]=", *name);
                        propertyValid = false;
                        break;
                    }
                }
            }
        }
        if (!propertyValid)
            return false;
    }

    for (TFieldIterator<UMapProperty> PropertyIterator(obj->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        UMapProperty* property = *PropertyIterator;
        bool propertyValid = true;
        if (property->HasAllPropertyFlags(CPF_SaveGame))
        {
            UProperty* keyProp = property->KeyProp;
            UProperty* valProp = property->ValueProp;
            bool isKeyObject = keyProp->IsA<UObjectPropertyBase>();
            bool isValObject = valProp->IsA<UObjectPropertyBase>();
            if (isKeyObject || isValObject)
            {
                UObjectPropertyBase* castedInnerProperty = Cast<UObjectPropertyBase>(keyProp);
                void* map = property->ContainerPtrToValuePtr<void>(obj);
                FScriptMapHelper MapHelper(property, map);
                for (int i = 0; i < MapHelper.Num(); i++)
                {
                    if (isKeyObject)
                    {
                        UObject* value = castedInnerProperty->GetObjectPropertyValue(MapHelper.GetKeyPtr(i));
                        if (!value) continue;

                        FString name = value->GetPathName();
                        if (!name.StartsWith(GetWorld()->GetPathName())) continue;

                        bool propertyElementValid = false;
                        for (AFGBuildable* otherBuilding : this->OriginalBuildings)
                        {
                            if (name.Contains(*(otherBuilding->GetPathName() + ".")) || value == otherBuilding)
                            {
                                propertyElementValid = true;
                                break;
                            }
                        }

                        if (!propertyElementValid)
                        {
                            SML::Logging::error(*property->GetPathName(), "[", i, "]=", *name);
                            propertyValid = false;
                            break;
                        }
                    }
                    if (isValObject)
                    {
                        UObject* value = castedInnerProperty->GetObjectPropertyValue(MapHelper.GetValuePtr(i));
                        if (!value) continue;

                        FString name = value->GetPathName();
                        if (!name.StartsWith(GetWorld()->GetPathName())) continue;

                        bool propertyElementValid = false;
                        for (AFGBuildable* otherBuilding : this->OriginalBuildings)
                        {
                            if (name.Contains(*(otherBuilding->GetPathName() + ".")) || value == otherBuilding)
                            {
                                propertyElementValid = true;
                                break;
                            }
                        }

                        if (!propertyElementValid)
                        {
                            SML::Logging::error(*property->GetPathName(), "[", i, "]=", *name);
                            propertyValid = false;
                            break;
                        }
                    }
                }
            }
        }
        if (!propertyValid)
            return false;
    }

    for (TFieldIterator<USetProperty> PropertyIterator(obj->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        USetProperty* property = *PropertyIterator;
        bool propertyValid = true;
        if (property->HasAllPropertyFlags(CPF_SaveGame))
        {
            UProperty* keyProp = property->ElementProp;
            if (keyProp->IsA<UObjectPropertyBase>())
            {
                UObjectPropertyBase* castedInnerProperty = Cast<UObjectPropertyBase>(keyProp);
                void* set = property->ContainerPtrToValuePtr<void>(obj);
                FScriptSetHelper SetHelper(property, set);
                for (int i = 0; i < SetHelper.Num(); i++)
                {
                    UObject* value = castedInnerProperty->GetObjectPropertyValue(SetHelper.GetElementPtr(i));
                    if (!value) continue;

                    FString name = value->GetPathName();
                    if (!name.StartsWith(GetWorld()->GetPathName())) continue;

                    bool propertyElementValid = false;
                    for (AFGBuildable* otherBuilding : this->OriginalBuildings)
                    {
                        if (name.Contains(*(otherBuilding->GetPathName() + ".")) || value == otherBuilding)
                        {
                            propertyElementValid = true;
                            break;
                        }
                    }

                    if (!propertyElementValid)
                    {
                        SML::Logging::error(*property->GetPathName(), "[", i, "]=", *name);
                        propertyValid = false;
                        break;
                    }
                }
            }
        }
        if (!propertyValid)
            return false;
    }
    return true;
}

void UAACopyBuildingsComponent::CalculateBounds()
{
    TMap<float, uint32> RotationCount;
    for(AActor* actor : this->OriginalBuildings)
        RotationCount.FindOrAdd(FGenericPlatformMath::Fmod(FGenericPlatformMath::Fmod(actor->GetActorRotation().Yaw, 90) + 90, 90))++;

    RotationCount.ValueSort([](const uint32& A, const uint32& B) {
        return A > B;
    });

    const FRotator Rotation = FRotator(0, (*RotationCount.CreateIterator()).Key, 0);

    FVector Min = FVector(TNumericLimits<float>::Max());
    FVector Max = FVector(-TNumericLimits<float>::Max());
    
    for(AFGBuildable* buildable : this->OriginalBuildings)
    {
        if(UShapeComponent* Clearance = buildable->GetClearanceComponent())
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
                    Min = Min.ComponentMin(Rotation.UnrotateVector(buildable->GetActorRotation().RotateVector(Box->GetComponentTransform().GetLocation() + Corner - buildable->GetActorLocation()) + buildable->GetActorLocation()));
                    Max = Max.ComponentMax(Rotation.UnrotateVector(buildable->GetActorRotation().RotateVector(Box->GetComponentTransform().GetLocation() + Corner - buildable->GetActorLocation()) + buildable->GetActorLocation()));
                }
            }
            else
            {
                // Are there any other types used as clearance?
            }
        }
        else
        {
            FActorSpawnParameters params;
            params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            params.bDeferConstruction = true;
            AFGBuildable* previewBuilding = this->GetWorld()->SpawnActor<AFGBuildable>(
                buildable->GetClass(), FTransform::Identity, params);
            previewBuilding->bDeferBeginPlay = true;
            previewBuilding->FinishSpawning(FTransform::Identity, true);
            FVector Origin;
            FVector Extents;
            previewBuilding->GetActorBounds(true, Origin, Extents);
            Extents = FVector(FGenericPlatformMath::RoundToFloat(Extents.X), FGenericPlatformMath::RoundToFloat(Extents.Y), FGenericPlatformMath::RoundToFloat(Extents.Z));

            for(int i = 0; i < (1 << 3); i++)
            {
                const int X = (i & 1) ? 1 : -1;
                const int Y = (i & 2) ? 1 : -1;
                const int Z = (i & 4) ? 1 : -1;
                FVector Corner = FVector(Extents.X * X, Extents.Y * Y, Extents.Z * Z);
                Min = Min.ComponentMin(Rotation.UnrotateVector(buildable->GetActorLocation() + buildable->GetActorRotation().RotateVector(Origin + Corner)));
                Max = Max.ComponentMax(Rotation.UnrotateVector(buildable->GetActorLocation() + buildable->GetActorRotation().RotateVector(Origin + Corner)));
            }
            previewBuilding->Destroy();
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
    for (AFGBuildable* buildable : this->OriginalBuildings)
    {
        if (!ValidateObject(buildable))
        {
            OutBuildingsWithIssues.Add(buildable);
            continue;
        }
        bool componentsValid = true;
        for (UActorComponent* component : buildable->GetComponents())
        {
            if (!ValidateObject(component))
            {
                componentsValid = false;
                break;
            }
        }
        if (!componentsValid)
        {
            OutBuildingsWithIssues.Add(buildable);
        }
    }
    return OutBuildingsWithIssues.Num() == 0;
}

void CopyBySerialization(UObject* from, UObject* to)
{
    TArray<uint8> serializedBytes;
    FMemoryWriter ActorWriter = FMemoryWriter(serializedBytes, true);
    FObjectAndNameAsStringProxyArchive Ar(ActorWriter, true);
    Ar.SetIsLoading(false);
    Ar.SetIsSaving(true);
    Ar.ArIsSaveGame = true;
    from->Serialize(Ar);

    FMemoryReader ActorReader = FMemoryReader(serializedBytes, true);
    FObjectAndNameAsStringProxyArchive Ar2(ActorReader, true);
    Ar2.SetIsLoading(true);
    Ar2.SetIsSaving(false);
    Ar2.ArIsSaveGame = true;
    to->Serialize(Ar2);
}

template <typename T>
T* GetObjectReplaceReference(T* Obj, T* From, T* To)
{
    if (Obj == From)
        return To;

    FString newName = Obj->GetPathName();
    if (!newName.Contains(*(From->GetPathName() + "."))) return nullptr;

    newName = newName.Replace(*(From->GetPathName() + "."), *(To->GetPathName() + "."));
    return FindObject<T>(nullptr, *newName);
}

void FixReferencesToBuilding(UObject* from, UObject* to, UObject* referenceFrom,
                                                        UObject* referenceTo)
{
    for (TFieldIterator<UObjectPropertyBase> PropertyIterator(from->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        UObjectPropertyBase* property = *PropertyIterator;
        if (property->HasAllPropertyFlags(CPF_SaveGame))
        {
            for (int i = 0; i < property->ArrayDim; i++)
            {
                void* originalValuePtr = property->ContainerPtrToValuePtr<void>(from, i);
                void* previewValuePtr = property->ContainerPtrToValuePtr<void>(to, i);
                UObject* original = property->GetObjectPropertyValue(originalValuePtr);

                if (!original) continue;

                FString originalName = original->GetPathName();
                FString newName = original->GetPathName();
                if (!originalName.Contains(*(referenceFrom->GetPathName() + ".")) && originalName != referenceFrom->
                    GetPathName()) continue;

                UObject* newObject = GetObjectReplaceReference<UObject>(original, referenceFrom, referenceTo);
                property->SetObjectPropertyValue(previewValuePtr, newObject);
            }
        }
    }

    for (TFieldIterator<UArrayProperty> PropertyIterator(from->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        UArrayProperty* property = *PropertyIterator;
        if (property->HasAllPropertyFlags(CPF_SaveGame))
        {
            UProperty* innerProperty = property->Inner;
            if (innerProperty->IsA<UObjectPropertyBase>())
            {
                UObjectPropertyBase* castedInnerProperty = Cast<UObjectPropertyBase>(innerProperty);
                void* originalArr = property->ContainerPtrToValuePtr<void>(from);
                void* previewArr = property->ContainerPtrToValuePtr<void>(to);
                FScriptArrayHelper OriginalArrayHelper(property, originalArr);
                FScriptArrayHelper PreviewArrayHelper(property, previewArr);
                PreviewArrayHelper.Resize(OriginalArrayHelper.Num());

                for (int i = 0; i < OriginalArrayHelper.Num(); i++)
                {
                    UObject* original = castedInnerProperty->GetObjectPropertyValue(OriginalArrayHelper.GetRawPtr(i));
                    if (!original) continue;

                    FString originalName = original->GetPathName();
                    FString newName = original->GetPathName();
                    if (!originalName.Contains(*(referenceFrom->GetPathName() + ".")) && originalName != referenceFrom->
                        GetPathName()) continue;

                    UObject* newObject = GetObjectReplaceReference<UObject>(original, referenceFrom, referenceTo);
                    castedInnerProperty->SetObjectPropertyValue(PreviewArrayHelper.GetRawPtr(i), newObject);
                }
            }
        }
    }

    for (TFieldIterator<UMapProperty> PropertyIterator(from->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        UMapProperty* property = *PropertyIterator;
        if (property->HasAllPropertyFlags(CPF_SaveGame))
        {
            UProperty* keyProp = property->KeyProp;
            UProperty* valProp = property->ValueProp;
            bool isKeyObject = keyProp->IsA<UObjectPropertyBase>();
            bool isValObject = valProp->IsA<UObjectPropertyBase>();
            if (isKeyObject || isValObject)
            {
                UObjectPropertyBase* castedInnerProperty = Cast<UObjectPropertyBase>(keyProp);
                void* originalMap = property->ContainerPtrToValuePtr<void>(from);
                void* previewMap = property->ContainerPtrToValuePtr<void>(to);
                FScriptMapHelper OriginalMapHelper(property, originalMap);
                FScriptMapHelper PreviewMapHelper(property, previewMap);
                for (int i = 0; i < OriginalMapHelper.Num(); i++)
                    PreviewMapHelper.AddPair(OriginalMapHelper.GetKeyPtr(i), OriginalMapHelper.GetValuePtr(i)); // Is this right?
                for (int i = 0; i < OriginalMapHelper.Num(); i++)
                {
                    if (isKeyObject)
                    {
                        UObject* original = castedInnerProperty->GetObjectPropertyValue(OriginalMapHelper.GetKeyPtr(i));
                        if (!original) continue;

                        FString originalName = original->GetPathName();
                        FString newName = original->GetPathName();
                        if (!originalName.Contains(*(referenceFrom->GetPathName() + ".")) && originalName !=
                            referenceFrom->GetPathName()) continue;

                        UObject* newObject = GetObjectReplaceReference<UObject>(original, referenceFrom, referenceTo);
                        castedInnerProperty->SetObjectPropertyValue(PreviewMapHelper.GetKeyPtr(i), newObject);
                    }
                    if (isValObject)
                    {
                        UObject* original = castedInnerProperty->GetObjectPropertyValue(
                            OriginalMapHelper.GetValuePtr(i));
                        if (!original) continue;

                        FString originalName = original->GetPathName();
                        FString newName = original->GetPathName();
                        if (!originalName.Contains(*(referenceFrom->GetPathName() + ".")) && originalName !=
                            referenceFrom->GetPathName()) continue;

                        UObject* newObject = GetObjectReplaceReference<UObject>(original, referenceFrom, referenceTo);
                        castedInnerProperty->SetObjectPropertyValue(PreviewMapHelper.GetValuePtr(i), newObject);
                    }
                }
                PreviewMapHelper.Rehash();
            }
        }
    }

    for (TFieldIterator<USetProperty> PropertyIterator(from->GetClass()); PropertyIterator; ++PropertyIterator)
    {
        USetProperty* property = *PropertyIterator;
        if (property->HasAllPropertyFlags(CPF_SaveGame))
        {
            UProperty* keyProp = property->ElementProp;
            if (keyProp->IsA<UObjectPropertyBase>())
            {
                UObjectPropertyBase* castedInnerProperty = Cast<UObjectPropertyBase>(keyProp);
                void* originalSet = property->ContainerPtrToValuePtr<void>(from);
                void* previewSet = property->ContainerPtrToValuePtr<void>(to);
                FScriptSetHelper OriginalSetHelper(property, originalSet);
                FScriptSetHelper PreviewSetHelper(property, previewSet);
                for (int i = 0; i < OriginalSetHelper.Num(); i++)
                    PreviewSetHelper.AddUninitializedValue();
                for (int i = 0; i < OriginalSetHelper.Num(); i++)
                {
                    UObject* original = castedInnerProperty->GetObjectPropertyValue(OriginalSetHelper.GetElementPtr(i));
                    if (!original) continue;

                    FString originalName = original->GetPathName();
                    FString newName = original->GetPathName();
                    if (!originalName.Contains(*(referenceFrom->GetPathName() + "."))
                        && originalName != referenceFrom->GetPathName()) continue;

                    UObject* newObject = GetObjectReplaceReference<UObject>(original, referenceFrom, referenceTo);
                    castedInnerProperty->SetObjectPropertyValue(PreviewSetHelper.GetElementPtr(i), newObject);
                }
                PreviewSetHelper.Rehash();
            }
        }
    }
}

void PreSaveGame(UObject* object)
{
    if (object->Implements<UFGSaveInterface>())
        IFGSaveInterface::Execute_PreSaveGame(
            object, UFGSaveSession::GetVersion(UFGSaveSession::Get(object->GetWorld())->mSaveHeader),
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
    for (AFGBuildable* buildable : this->OriginalBuildings)
    {
        FTransform newTransform = TransformAroundPoint(buildable->GetActorTransform(), Relative ? BuildingsBounds.Rotation.RotateVector(Offset) : Offset, Rotation, RotationCenter);
        // TODO: rotation around a point
        FActorSpawnParameters params;
        params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        params.bDeferConstruction = true;
        AFGBuildable* previewBuilding = this->GetWorld()->SpawnActor<AFGBuildable>(
            buildable->GetClass(), newTransform, params);
        previewBuilding->bDeferBeginPlay = true;
        previewBuilding->FinishSpawning(FTransform::Identity, true);
        this->BuildingsPreview.FindOrAdd(buildable).Buildings.Add(CurrentId, previewBuilding);

        for (UActorComponent* newComponent : previewBuilding->GetComponents())
            PreLoadGame(newComponent);
        PreLoadGame(previewBuilding);

        for (UActorComponent* component : buildable->GetComponents())
            PreSaveGame(component);
        PreSaveGame(buildable);
    }
    for (int i = 0; i < this->OriginalBuildings.Num(); i++)
    {
        AFGBuildable* buildable = this->OriginalBuildings[i];
        AFGBuildable* previewBuilding = this->BuildingsPreview.Find(buildable)->Buildings[CurrentId];

        CopyBySerialization(buildable, previewBuilding);

        for (UActorComponent* component : buildable->GetComponents())
        {
            if (!component->Implements<UFGSaveInterface>()) continue;
            UActorComponent* previewComponent = nullptr;
            for (UActorComponent* newComponent : previewBuilding->GetComponents())
                if (newComponent->GetName() == component->GetName())
                {
                    previewComponent = newComponent;
                    break;
                }

            if (!previewComponent)
            {
                previewComponent = NewObject<UActorComponent>(
                    GetObjectReplaceReference<UObject>(component->GetOuter(), buildable, previewBuilding),
                    component->GetClass(), FName(*component->GetName()), component->GetFlags());
                previewComponent->RegisterComponent();
                if (previewComponent->IsA<USceneComponent>())
                    SML::Logging::fatal(*FString::Printf(
                        TEXT("Component %s of %s is a scene component. Not implemented yet!"), *component->GetName(),
                        *buildable->GetName())); // will this ever happen?
                // TODO: SceneComponents?
            }

            CopyBySerialization(component, previewComponent);
        }
    }

    for (int i = 0; i < this->OriginalBuildings.Num(); i++)
    {
        AFGBuildable* buildable = this->OriginalBuildings[i];
        AFGBuildable* previewBuilding = this->BuildingsPreview.Find(buildable)->Buildings[CurrentId];
        for (int j = 0; j < this->OriginalBuildings.Num(); j++)
        {
            AFGBuildable* otherBuildable = this->OriginalBuildings[j];
            AFGBuildable* otherPreviewBuilding = this->BuildingsPreview.Find(otherBuildable)->Buildings[CurrentId];
            FixReferencesToBuilding(otherBuildable, otherPreviewBuilding, buildable, previewBuilding);

            for (UActorComponent* component : otherBuildable->GetComponents())
            {
                if (!component->Implements<UFGSaveInterface>()) continue;
                UActorComponent* previewComponent = nullptr;
                for (UActorComponent* newComponent : otherPreviewBuilding->GetComponents())
                {
                    if (newComponent->GetName() == component->GetName())
                    {
                        previewComponent = newComponent;
                        break;
                    }
                }

                if (!previewComponent)
                {
                    SML::Logging::fatal(*FString::Printf(
                        TEXT("Component %s of %s does not exist. This shouldn't happen!"), *component->GetName(),
                        *previewBuilding->GetName())); // I would like SML::Logging::wtf
                    continue;
                }

                FixReferencesToBuilding(component, previewComponent, buildable, previewBuilding);
            }
        }
    }

    for (int i = 0; i < this->OriginalBuildings.Num(); i++)
    {
        AFGBuildable* buildable = this->OriginalBuildings[i];
        AFGBuildable* previewBuilding = this->BuildingsPreview.Find(buildable)->Buildings[CurrentId];
        for (UActorComponent* newComponent : previewBuilding->GetComponents())
            PostLoadGame(newComponent);
        PostLoadGame(previewBuilding);

        for (UActorComponent* component : buildable->GetComponents())
            PostSaveGame(component);
        PostSaveGame(buildable);
    }

    for (int i = 0; i < this->OriginalBuildings.Num(); i++)
    {
        AFGBuildable* buildable = this->OriginalBuildings[i];
        AFGBuildable* previewBuilding = this->BuildingsPreview.Find(buildable)->Buildings[CurrentId];
        previewBuilding->DeferredBeginPlay();
        TArray<UFGColoredInstanceMeshProxy*> ColoredInstanceMeshProxies;
        previewBuilding->GetComponents<UFGColoredInstanceMeshProxy>(ColoredInstanceMeshProxies);
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
    for (AFGBuildable* buildable : this->OriginalBuildings)
    {
        FTransform newTransform = TransformAroundPoint(buildable->GetActorTransform(), Relative ? BuildingsBounds.Rotation.RotateVector(Offset) : Offset, Rotation, RotationCenter);
        AFGBuildable* previewBuilding = this->BuildingsPreview[buildable].Buildings[CopyId];
        const EComponentMobility::Type CurrentMobility = previewBuilding->GetRootComponent()->Mobility;
        previewBuilding->GetRootComponent()->SetMobility(EComponentMobility::Movable);
        previewBuilding->SetActorTransform(newTransform);
        previewBuilding->GetRootComponent()->SetMobility(CurrentMobility);
    }
}

void UAACopyBuildingsComponent::RemoveCopy(int CopyId)
{
    for (AFGBuildable* buildable : this->OriginalBuildings)
    {
        AFGBuildable* previewBuilding = this->BuildingsPreview[buildable].Buildings[CopyId];
        this->BuildingsPreview[buildable].Buildings.Remove(CopyId);
        previewBuilding->Destroy();
    }
}

void UAACopyBuildingsComponent::Finish()
{
    TArray<AFGBuildable*> previewBuildingKeys;
    this->BuildingsPreview.GetKeys(previewBuildingKeys);
    for (AFGBuildable* buildable : previewBuildingKeys)
    {
        for (const auto Preview : this->BuildingsPreview[buildable].Buildings)
        {
            AFGBuildable* previewBuilding = Preview.Value;
            TArray<UFGColoredInstanceMeshProxy*> ColoredInstanceMeshProxies;
            previewBuilding->GetComponents<UFGColoredInstanceMeshProxy>(ColoredInstanceMeshProxies);
            for (UFGColoredInstanceMeshProxy* Mesh : ColoredInstanceMeshProxies)
            {
                Mesh->SetInstanced(true);
                UFGColoredInstanceMeshProxy* OriginalMeshComponent = nullptr;
                TArray<UFGColoredInstanceMeshProxy*> OriginalColoredInstanceMeshProxies;
                previewBuilding->GetComponents<UFGColoredInstanceMeshProxy>(OriginalColoredInstanceMeshProxies);
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
                        *Mesh->GetName(), *buildable->GetName())); // I would like SML::Logging::wtf
                    continue;
                }
                
                for (int MatNum = 0; MatNum < Mesh->GetNumMaterials(); MatNum++)
                    Mesh->SetMaterial(MatNum, OriginalMeshComponent->GetMaterial(MatNum));
            }
        }
    }
}
#pragma optimize( "", on )