// Fill out your copyright notice in the Description page of Project Settings.


#include "AACopyBuildingsComponent.h"

#include "SML/util/Logging.h"
#include "AAEquipment.h"
#include "FGBuildableConveyorBelt.h"
#include "FGBuildableStorage.h"
#include "GameFramework/Actor.h"
#pragma optimize( "", off )

UAACopyBuildingsComponent::UAACopyBuildingsComponent() {
	ValidCheckSkipProperties.Add(AActor::StaticClass()->FindPropertyByName(TEXT("Owner")));
}

bool UAACopyBuildingsComponent::SetActors(TArray<AActor*>& actors, TArray<AFGBuildable*>& out_buildingsWithIssues) {
	TArray<AFGBuildable*> buildings;
	for (AActor* actor : actors) {
		if (actor->IsA<AFGBuildable>()) {
			buildings.Add(static_cast<AFGBuildable*>(actor));
		}
	}
	return SetBuildings(buildings, out_buildingsWithIssues);
}

bool UAACopyBuildingsComponent::SetBuildings(TArray<AFGBuildable*>& buildings, TArray<AFGBuildable*>& out_buildingsWithIssues) {
	this->mOriginalBuildings = buildings;
	Algo::Sort(this->mOriginalBuildings, [](AFGBuildable* a, AFGBuildable* b) {
		return a->GetBuildTime() < b->GetBuildTime();
	});
	return ValidateBuildings(out_buildingsWithIssues);
}

bool UAACopyBuildingsComponent::ValidateObject(UObject* obj) {
	for (TFieldIterator<UObjectPropertyBase> PropertyIterator(obj->GetClass()); PropertyIterator; ++PropertyIterator) {
		UObjectPropertyBase* property = *PropertyIterator;
		bool propertyValid = true;
		if (property->HasAllPropertyFlags(CPF_SaveGame)) {
			for (int i = 0; i < property->ArrayDim; i++) {
				void* valuePtr = property->ContainerPtrToValuePtr<void>(obj, i);
				UObject* value = property->GetObjectPropertyValue(valuePtr);

				if (!value) continue;

				FString name = value->GetPathName();
				if (!name.StartsWith(GetWorld()->GetPathName())) continue;

				bool propertyElementValid = false;
				for (AFGBuildable* otherBuilding : this->mOriginalBuildings) {
					if (name.Contains(*(otherBuilding->GetPathName() + ".")) || value == otherBuilding) {
						propertyElementValid = true;
						break;
					}
				}

				if (!propertyElementValid) {
					SML::Logging::error(*property->GetPathName(), "[", i, "]=", *name);
					propertyValid = false;
					break;
				}
			}
		}
		if (!propertyValid) {
			return false;
		}
	}

	for (TFieldIterator<UArrayProperty> PropertyIterator(obj->GetClass()); PropertyIterator; ++PropertyIterator) {
		UArrayProperty* property = *PropertyIterator;
		bool propertyValid = true;
		if (property->HasAllPropertyFlags(CPF_SaveGame))
		{
			UProperty* innerProperty = property->Inner;
			if (innerProperty->IsA<UObjectPropertyBase>()) {
				UObjectPropertyBase* castedInnerProperty = Cast<UObjectPropertyBase>(innerProperty);
				void* arr = property->ContainerPtrToValuePtr<void>(obj);
				FScriptArrayHelper ArrayHelper(property, arr);
				
				for (int i = 0; i < ArrayHelper.Num(); i++) {
					UObject* value = castedInnerProperty->GetObjectPropertyValue(ArrayHelper.GetRawPtr(i));
					if (!value) continue;

					FString name = value->GetPathName();
					if (!name.StartsWith(GetWorld()->GetPathName())) continue;

					bool propertyElementValid = false;
					for (AFGBuildable* otherBuilding : this->mOriginalBuildings) {
						if (name.Contains(*(otherBuilding->GetPathName() + ".")) || value == otherBuilding) {
							propertyElementValid = true;
							break;
						}
					}

					if (!propertyElementValid) {
						SML::Logging::error(*property->GetPathName(), "[", i, "]=", *name);
						propertyValid = false;
						break;
					}
				}
			}
		}
		if (!propertyValid) {
			return false;
		}
	}

	for (TFieldIterator<UMapProperty> PropertyIterator(obj->GetClass()); PropertyIterator; ++PropertyIterator) {
		UMapProperty* property = *PropertyIterator;
		bool propertyValid = true;
		if (property->HasAllPropertyFlags(CPF_SaveGame))
		{
			UProperty* keyProp = property->KeyProp;
			UProperty* valProp = property->ValueProp;
			bool isKeyObject = keyProp->IsA<UObjectPropertyBase>();
			bool isValObject = valProp->IsA<UObjectPropertyBase>();
			if (isKeyObject || isValObject) {
				UObjectPropertyBase* castedInnerProperty = Cast<UObjectPropertyBase>(keyProp);
				void* map = property->ContainerPtrToValuePtr<void>(obj);
				FScriptMapHelper MapHelper(property, map);		
				for (int i = 0; i < MapHelper.Num(); i++) {
					if(isKeyObject)
					{
						UObject* value = castedInnerProperty->GetObjectPropertyValue(MapHelper.GetKeyPtr(i));
						if (!value) continue;

						FString name = value->GetPathName();
						if (!name.StartsWith(GetWorld()->GetPathName())) continue;

						bool propertyElementValid = false;
						for (AFGBuildable* otherBuilding : this->mOriginalBuildings) {
							if (name.Contains(*(otherBuilding->GetPathName() + ".")) || value == otherBuilding) {
								propertyElementValid = true;
								break;
							}
						}

						if (!propertyElementValid) {
							SML::Logging::error(*property->GetPathName(), "[", i, "]=", *name);
							propertyValid = false;
							break;
						}
					}
					if(isValObject)
					{
						UObject* value = castedInnerProperty->GetObjectPropertyValue(MapHelper.GetValuePtr(i));
						if (!value) continue;

						FString name = value->GetPathName();
						if (!name.StartsWith(GetWorld()->GetPathName())) continue;

						bool propertyElementValid = false;
						for (AFGBuildable* otherBuilding : this->mOriginalBuildings) {
							if (name.Contains(*(otherBuilding->GetPathName() + ".")) || value == otherBuilding) {
								propertyElementValid = true;
								break;
							}
						}

						if (!propertyElementValid) {
							SML::Logging::error(*property->GetPathName(), "[", i, "]=", *name);
							propertyValid = false;
							break;
						}
					}
				}
			}
		}
		if (!propertyValid) {
			return false;
		}
	}

	for (TFieldIterator<USetProperty> PropertyIterator(obj->GetClass()); PropertyIterator; ++PropertyIterator) {
		USetProperty* property = *PropertyIterator;
		bool propertyValid = true;
		if (property->HasAllPropertyFlags(CPF_SaveGame))
		{
			UProperty* keyProp = property->ElementProp;
			if (keyProp->IsA<UObjectPropertyBase>()) {
				UObjectPropertyBase* castedInnerProperty = Cast<UObjectPropertyBase>(keyProp);
				void* set = property->ContainerPtrToValuePtr<void>(obj);
				FScriptSetHelper SetHelper(property, set);
				for (int i = 0; i < SetHelper.Num(); i++) {
					UObject* value = castedInnerProperty->GetObjectPropertyValue(SetHelper.GetElementPtr(i));
					if (!value) continue;

					FString name = value->GetPathName();
					if (!name.StartsWith(GetWorld()->GetPathName())) continue;

					bool propertyElementValid = false;
					for (AFGBuildable* otherBuilding : this->mOriginalBuildings) {
						if (name.Contains(*(otherBuilding->GetPathName() + ".")) || value == otherBuilding) {
							propertyElementValid = true;
							break;
						}
					}

					if (!propertyElementValid) {
						SML::Logging::error(*property->GetPathName(), "[", i, "]=", *name);
						propertyValid = false;
						break;
					}
				}
			}
		}
		if (!propertyValid) {
			return false;
		}
	}
	return true;
}

bool UAACopyBuildingsComponent::ValidateBuildings(TArray<AFGBuildable*>& out_buildingsWithIssues) {
	for (AFGBuildable* buildable : this->mOriginalBuildings) {
		if (!ValidateObject(buildable)) {
			out_buildingsWithIssues.Add(buildable);
			continue;
		}
		bool componentsValid = true;
		for (UActorComponent* component : buildable->GetComponents()) {
			if (!ValidateObject(component)) {
				componentsValid = false;
				break;
			}
		}
		if (!componentsValid) {
			out_buildingsWithIssues.Add(buildable);
		}
	}
	return out_buildingsWithIssues.Num() == 0;
}

void CopyBySerialization(UObject* from, UObject* to) {
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

template<typename T>
T* GetObjectReplaceReference(T* Obj, T* From, T* To)
{
	if(Obj == From)
		return To;
	
	FString newName = Obj->GetPathName();
	if (!newName.Contains(*(From->GetPathName() + "."))) return nullptr;

	newName = newName.Replace(*(From->GetPathName() + "."), *(To->GetPathName() + "."));
	return FindObject<T>(nullptr, *newName);
}

void UAACopyBuildingsComponent::FixReferencesToBuilding(UObject* from, UObject* to, UObject* referenceFrom, UObject* referenceTo) const
{
	for (TFieldIterator<UObjectPropertyBase> PropertyIterator(from->GetClass()); PropertyIterator; ++PropertyIterator) {
		UObjectPropertyBase* property = *PropertyIterator;
		if (property->HasAllPropertyFlags(CPF_SaveGame)) {
			for (int i = 0; i < property->ArrayDim; i++) {
				void* originalValuePtr = property->ContainerPtrToValuePtr<void>(from, i);
				void* previewValuePtr = property->ContainerPtrToValuePtr<void>(to, i);
				UObject* original = property->GetObjectPropertyValue(originalValuePtr);

				if (!original) continue;

				FString originalName = original->GetPathName();
				FString newName = original->GetPathName();
				if (!originalName.Contains(*(referenceFrom->GetPathName() + ".")) && originalName != referenceFrom->GetPathName()) continue;

				UObject* newObject = GetObjectReplaceReference<UObject>(original, referenceFrom, referenceTo);
				property->SetObjectPropertyValue(previewValuePtr, newObject);
			}
		}
	}

	for (TFieldIterator<UArrayProperty> PropertyIterator(from->GetClass()); PropertyIterator; ++PropertyIterator) {
		UArrayProperty* property = *PropertyIterator;
		if (property->HasAllPropertyFlags(CPF_SaveGame))
		{
			UProperty* innerProperty = property->Inner;
			if (innerProperty->IsA<UObjectPropertyBase>()) {
				UObjectPropertyBase* castedInnerProperty = Cast<UObjectPropertyBase>(innerProperty);
				void* originalArr = property->ContainerPtrToValuePtr<void>(from);
				void* previewArr = property->ContainerPtrToValuePtr<void>(to);
				FScriptArrayHelper OriginalArrayHelper(property, originalArr);		
				FScriptArrayHelper PreviewArrayHelper(property, previewArr);	
				PreviewArrayHelper.Resize(OriginalArrayHelper.Num());
				
				for (int i = 0; i < OriginalArrayHelper.Num(); i++) {				
					UObject* original = castedInnerProperty->GetObjectPropertyValue(OriginalArrayHelper.GetRawPtr(i));
					if (!original) continue;

					FString originalName = original->GetPathName();
					FString newName = original->GetPathName();
					if (!originalName.Contains(*(referenceFrom->GetPathName() + ".")) && originalName != referenceFrom->GetPathName()) continue;

					UObject* newObject = GetObjectReplaceReference<UObject>(original, referenceFrom, referenceTo);
					castedInnerProperty->SetObjectPropertyValue(PreviewArrayHelper.GetRawPtr(i), newObject);
				}
			}
		}
	}

	for (TFieldIterator<UMapProperty> PropertyIterator(from->GetClass()); PropertyIterator; ++PropertyIterator) {
		UMapProperty* property = *PropertyIterator;
		if (property->HasAllPropertyFlags(CPF_SaveGame))
		{
			UProperty* keyProp = property->KeyProp;
			UProperty* valProp = property->ValueProp;
			bool isKeyObject = keyProp->IsA<UObjectPropertyBase>();
			bool isValObject = valProp->IsA<UObjectPropertyBase>();
			if (isKeyObject || isValObject) {
				UObjectPropertyBase* castedInnerProperty = Cast<UObjectPropertyBase>(keyProp);
				void* originalMap = property->ContainerPtrToValuePtr<void>(from);
				void* previewMap = property->ContainerPtrToValuePtr<void>(to);
				FScriptMapHelper OriginalMapHelper(property, originalMap);		
				FScriptMapHelper PreviewMapHelper(property, previewMap);
				for(int i = 0; i < OriginalMapHelper.Num(); i++)
					PreviewMapHelper.AddPair(OriginalMapHelper.GetKeyPtr(i), OriginalMapHelper.GetValuePtr(i)); // Is this right?
				for (int i = 0; i < OriginalMapHelper.Num(); i++) {
					if(isKeyObject)
					{
						UObject* original = castedInnerProperty->GetObjectPropertyValue(OriginalMapHelper.GetKeyPtr(i));
						if (!original) continue;

						FString originalName = original->GetPathName();
						FString newName = original->GetPathName();
						if (!originalName.Contains(*(referenceFrom->GetPathName() + ".")) && originalName != referenceFrom->GetPathName()) continue;

						UObject* newObject = GetObjectReplaceReference<UObject>(original, referenceFrom, referenceTo);
						castedInnerProperty->SetObjectPropertyValue(PreviewMapHelper.GetKeyPtr(i), newObject);
					}
					if(isValObject)
					{
						UObject* original = castedInnerProperty->GetObjectPropertyValue(OriginalMapHelper.GetValuePtr(i));
						if (!original) continue;

						FString originalName = original->GetPathName();
						FString newName = original->GetPathName();
						if (!originalName.Contains(*(referenceFrom->GetPathName() + ".")) && originalName != referenceFrom->GetPathName()) continue;

						UObject* newObject = GetObjectReplaceReference<UObject>(original, referenceFrom, referenceTo);
						castedInnerProperty->SetObjectPropertyValue(PreviewMapHelper.GetValuePtr(i), newObject);
					}
				}
				PreviewMapHelper.Rehash();
			}
		}
	}

	for (TFieldIterator<USetProperty> PropertyIterator(from->GetClass()); PropertyIterator; ++PropertyIterator) {
		USetProperty* property = *PropertyIterator;
		if (property->HasAllPropertyFlags(CPF_SaveGame))
		{
			UProperty* keyProp = property->ElementProp;
			if (keyProp->IsA<UObjectPropertyBase>()) {
				UObjectPropertyBase* castedInnerProperty = Cast<UObjectPropertyBase>(keyProp);
				void* originalSet = property->ContainerPtrToValuePtr<void>(from);
				void* previewSet = property->ContainerPtrToValuePtr<void>(to);
				FScriptSetHelper OriginalSetHelper(property, originalSet);		
				FScriptSetHelper PreviewSetHelper(property, previewSet);
				for(int i = 0; i < OriginalSetHelper.Num(); i++)
					PreviewSetHelper.AddUninitializedValue();
				for (int i = 0; i < OriginalSetHelper.Num(); i++) {
					UObject* original = castedInnerProperty->GetObjectPropertyValue(OriginalSetHelper.GetElementPtr(i));
					if (!original) continue;

					FString originalName = original->GetPathName();
					FString newName = original->GetPathName();
					if (!originalName.Contains(*(referenceFrom->GetPathName() + ".")) && originalName != referenceFrom->GetPathName()) continue;

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
	if(object->Implements<UFGSaveInterface>())
		IFGSaveInterface::Execute_PreSaveGame(object, UFGSaveSession::GetVersion(UFGSaveSession::Get(object->GetWorld())->mSaveHeader), FEngineVersion::Current().GetChangelist());
}

void PostSaveGame(UObject* object)
{
	if(object->Implements<UFGSaveInterface>())
		IFGSaveInterface::Execute_PostSaveGame(object, UFGSaveSession::GetVersion(UFGSaveSession::Get(object->GetWorld())->mSaveHeader), FEngineVersion::Current().GetChangelist());
}

void PreLoadGame(UObject* object)
{
	if(object->Implements<UFGSaveInterface>())
		IFGSaveInterface::Execute_PreLoadGame(object, UFGSaveSession::GetVersion(UFGSaveSession::Get(object->GetWorld())->mSaveHeader), FEngineVersion::Current().GetChangelist());
}

void PostLoadGame(UObject* object)
{
	if(object->Implements<UFGSaveInterface>())
		IFGSaveInterface::Execute_PostLoadGame(object, UFGSaveSession::GetVersion(UFGSaveSession::Get(object->GetWorld())->mSaveHeader), FEngineVersion::Current().GetChangelist());
}

int UAACopyBuildingsComponent::AddCopy(FVector offset, FRotator rotation) {
	this->mIdIdx.Add(mCurrentId);
	this->mCopyLocations.Add(TPair<FVector, FRotator>(offset, rotation));
	for (AFGBuildable* buildable : this->mOriginalBuildings) {
		FVector newLocation = buildable->GetActorLocation() + offset;
		FRotator newRotation = buildable->GetActorRotation() + rotation;
		// TODO: rotation around a point
		// TODO: hologram material
		FActorSpawnParameters params;
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		params.bDeferConstruction = true;
		AFGBuildable* previewBuilding = this->GetWorld()->SpawnActor<AFGBuildable>(buildable->GetClass(), FTransform(newRotation, newLocation, buildable->GetActorScale3D()), params);
		previewBuilding->bDeferBeginPlay = true;
		previewBuilding->FinishSpawning(FTransform::Identity, true);
		this->mBuildingsPreview.FindOrAdd(buildable).mBuildings.Add(previewBuilding);

		for (UActorComponent* newComponent : previewBuilding->GetComponents())
			PreLoadGame(newComponent);
		PreLoadGame(previewBuilding);
		
		for (UActorComponent* component : buildable->GetComponents())
			PreSaveGame(component);
		PreSaveGame(buildable);
	}
	for (int i = 0; i < this->mOriginalBuildings.Num(); i++) {
		AFGBuildable* buildable = this->mOriginalBuildings[i];
		AFGBuildable* previewBuilding = this->mBuildingsPreview.Find(buildable)->mBuildings.Last();

		CopyBySerialization(buildable, previewBuilding);
		
		for (UActorComponent* component : buildable->GetComponents()) {
			if(!component->Implements<UFGSaveInterface>()) continue;
			UActorComponent* previewComponent = nullptr;
			for (UActorComponent* newComponent : previewBuilding->GetComponents()) {
				if (newComponent->GetName() == component->GetName()) {
					previewComponent = newComponent;
					break;
				}
			}

			if (!previewComponent)
			{
				previewComponent = NewObject<UActorComponent>(GetObjectReplaceReference<UObject>(component->GetOuter(), buildable, previewBuilding), component->GetClass(), FName(*component->GetName()), component->GetFlags());
				previewComponent->RegisterComponent();
				if (previewComponent->IsA<USceneComponent>())
					SML::Logging::fatal(*FString::Printf(TEXT("Component %s of %s is a scene component. Not implemented yet!"), *component->GetName(), *buildable->GetName())); // will this ever happen?
				// TODO: SceneComponents?
			}

			CopyBySerialization(component, previewComponent);
		}
	}
	
	for (int i = 0; i < this->mOriginalBuildings.Num(); i++)
	{
		AFGBuildable* buildable = this->mOriginalBuildings[i];
		AFGBuildable* previewBuilding = this->mBuildingsPreview.Find(buildable)->mBuildings.Last();
		for (int j = 0; j < this->mOriginalBuildings.Num(); j++) {
			AFGBuildable* otherBuildable = this->mOriginalBuildings[j];
			AFGBuildable* otherPreviewBuilding = this->mBuildingsPreview.Find(otherBuildable)->mBuildings.Last();
			FixReferencesToBuilding(otherBuildable, otherPreviewBuilding, buildable, previewBuilding);

			for (UActorComponent* component : otherBuildable->GetComponents()) {
				if(!component->Implements<UFGSaveInterface>()) continue;
				UActorComponent* previewComponent = nullptr;
				for (UActorComponent* newComponent : otherPreviewBuilding->GetComponents()) {
					if (newComponent->GetName() == component->GetName()) {
						previewComponent = newComponent;
						break;
					}
				}

				if (!previewComponent)
				{
					SML::Logging::fatal(*FString::Printf(TEXT("Component %s of %s does not exist. This shouldn't happen!"), *component->GetName(), *previewBuilding->GetName())); // I would like SML::Logging::wtf
					continue;
				}

				FixReferencesToBuilding(component, previewComponent, buildable, previewBuilding);
			}
		}
	}
	
	for (int i = 0; i < this->mOriginalBuildings.Num(); i++)
	{
		AFGBuildable* buildable = this->mOriginalBuildings[i];
		AFGBuildable* previewBuilding = this->mBuildingsPreview.Find(buildable)->mBuildings.Last();
		for (UActorComponent* newComponent : previewBuilding->GetComponents())
			PostLoadGame(newComponent);
		PostLoadGame(previewBuilding);
		
		for (UActorComponent* component : buildable->GetComponents())
			PostSaveGame(component);
		PostSaveGame(buildable);
	}
	
	for (int i = 0; i < this->mOriginalBuildings.Num(); i++)
	{
		AFGBuildable* buildable = this->mOriginalBuildings[i];
		AFGBuildable* previewBuilding = this->mBuildingsPreview.Find(buildable)->mBuildings.Last();
		previewBuilding->DeferredBeginPlay();
	}
	return mCurrentId++;
}

void UAACopyBuildingsComponent::RemoveCopy(int copyId) {
	int pos = this->mIdIdx.Find(copyId);
	if (pos == INDEX_NONE) {
		return;
	}
	TArray<AFGBuildable*> previewBuildingKeys;
	this->mBuildingsPreview.GetKeys(previewBuildingKeys);
	for (AFGBuildable* buildable : previewBuildingKeys) {
		AFGBuildable* previewActor = this->mBuildingsPreview[buildable].mBuildings[pos];
		this->mBuildingsPreview[buildable].mBuildings.RemoveAt(pos);
		previewActor->Destroy();
	}
	this->mCopyLocations.RemoveAt(pos);
	this->mIdIdx.RemoveAt(pos);
}

void UAACopyBuildingsComponent::Finish() {
	// TODO: Preview should be holograms
	/*
	TArray<AFGBuildable*> previewBuildingKeys;
	this->mBuildingsPreview.GetKeys(previewBuildingKeys);
	for (AFGBuildable* buildable : previewBuildingKeys) {
		for (int i = this->mBuildingsPreview[buildable].mBuildings.Num() - 1; i >= 0; i--) {
			AFGBuildable* previewActor = this->mBuildingsPreview[buildable].mBuildings[i];
			this->mBuildingsPreview[buildable].mBuildings.RemoveAt(i);
			previewActor->Destroy();
		}
	}
	*/
}
#pragma optimize( "", on )