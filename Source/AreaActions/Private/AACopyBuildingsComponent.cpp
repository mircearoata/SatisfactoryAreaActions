// Fill out your copyright notice in the Description page of Project Settings.


#include "AACopyBuildingsComponent.h"
#include "SML/util/Logging.h"
#include "AAEquipment.h"
#include "Engine/Engine.h"
#include "FGBuildableConveyorBelt.h"
#include "GameFramework/Actor.h"
#pragma optimize( "", off )

void UAACopyBuildingsComponent::SetActors(TArray<AActor*>& actors) {
	TArray<AFGBuildable*> buildings;
	for (AActor* actor : actors) {
		if (actor->IsA<AFGBuildable>()) {
			buildings.Add((AFGBuildable*)actor);
		}
	}
	SetBuildings(buildings);
}

void UAACopyBuildingsComponent::SetBuildings(TArray<AFGBuildable*>& buildings) {
	this->mOriginalBuildings = buildings;
	Algo::Sort(this->mOriginalBuildings, [](AFGBuildable* a, AFGBuildable* b) {
		return a->GetBuildTime() < b->GetBuildTime();
	});
}

void CopyBySerialization(UObject* from, UObject* to) {
	TArray<uint8> serializedBytes;
	FMemoryWriter ActorWriter = FMemoryWriter(serializedBytes, true);
	FObjectAndNameAsStringProxyArchive Ar(ActorWriter, true);
	//Ar.ArIsSaveGame = true;
	Ar.SetIsLoading(false);
	Ar.SetIsSaving(true);
	from->Serialize(Ar);

	FMemoryReader ActorReader = FMemoryReader(serializedBytes, true);
	FObjectAndNameAsStringProxyArchive Ar2(ActorReader, true);
	//Ar2.ArIsSaveGame = true;
	Ar2.SetIsLoading(true);
	Ar2.SetIsSaving(false);
	to->Serialize(Ar2);
}

int UAACopyBuildingsComponent::AddCopy(FVector offset, FRotator rotation) {
	this->mIdIdx.Add(mCurrentId);
	this->mCopyLocations.Add(TPair<FVector, FRotator>(offset, rotation));
	FString worldPathName = GetWorld()->GetPathName();
	SML::Logging::warning(*worldPathName);
	for (AFGBuildable* buildable : this->mOriginalBuildings) {
		FVector newLocation = buildable->GetActorLocation() + offset;
		FRotator newRotation = buildable->GetActorRotation() + rotation;
		// TODO: rotation around a point
		// TODO: replace with hologram, and move this entire thing to Finish
		AFGBuildable* previewBuilding = AFGBuildableSubsystem::Get(this)->BeginSpawnBuildable(buildable->GetClass(), FTransform(newRotation, newLocation, FVector::OneVector));
		previewBuilding->bDeferBeginPlay = true;
		this->mBuildingsPreview.FindOrAdd(buildable).mBuildings.Add(previewBuilding);
	}
	for (int i = 0; i < this->mOriginalBuildings.Num(); i++) {
		AFGBuildable* buildable = this->mOriginalBuildings[i];
		AFGBuildable* previewBuilding = this->mBuildingsPreview.Find(buildable)->mBuildings.Last();

		TMap<UObjectProperty*, UObject*> valuesBeforeCopy;
		for (TFieldIterator<UObjectProperty> PropertyIterator(buildable->GetClass()); PropertyIterator; ++PropertyIterator) {
			UObjectProperty* property = *PropertyIterator;
			if (property->IsA<UClassProperty>()) continue;

			if (!property->HasAllPropertyFlags(EPropertyFlags::CPF_SaveGame)) {
				void* previewValuePtr = property->ContainerPtrToValuePtr<void>(previewBuilding);
				UObject* currentValue = property->GetPropertyValue(previewValuePtr);
				valuesBeforeCopy.FindOrAdd(property) = currentValue;
			}
		}
		CopyBySerialization(buildable, previewBuilding);
		for (TFieldIterator<UObjectProperty> PropertyIterator(buildable->GetClass()); PropertyIterator; ++PropertyIterator) {
			UObjectProperty* property = *PropertyIterator;
			if (property->IsA<UClassProperty>()) continue;

			void* originalValuePtr = property->ContainerPtrToValuePtr<void>(buildable);
			void* previewValuePtr = property->ContainerPtrToValuePtr<void>(previewBuilding);
			
			if (property->HasAllPropertyFlags(EPropertyFlags::CPF_SaveGame)) {
				UObject* original = property->GetPropertyValue(originalValuePtr);
				if (!original) continue;

				FString originalName = original->GetPathName();
				FString newName = original->GetPathName();
				for (int j = 0; j <= i; j++) {
					AFGBuildable* origBuilding = this->mOriginalBuildings[j];
					newName = newName.Replace(*(origBuilding->GetPathName() + "."), *(this->mBuildingsPreview.Find(origBuilding)->mBuildings.Last()->GetPathName() + "."));
				}

				UObject* newObject = FindObject<UObject>(nullptr, *newName);
				property->SetPropertyValue(previewValuePtr, newObject);
			}
			else {
				UObject** previousValuePtr = valuesBeforeCopy.Find(property);
				UObject* current = property->GetPropertyValue(previewValuePtr);
				if (previousValuePtr) {
					// only reset references to non static stuff
					if ((!(*previousValuePtr) && current->GetPathName().StartsWith(GetWorld()->GetPathName())) || ((*previousValuePtr) && (*previousValuePtr)->GetPathName().StartsWith(GetWorld()->GetPathName()))) {
						property->SetPropertyValue(previewValuePtr, *previousValuePtr);
					}
				}
			}
		}
		previewBuilding->FinishSpawning(FTransform::Identity, true);
		for (UActorComponent* component : buildable->GetComponents()) {
			UActorComponent* previewComponent = nullptr;
			for (UActorComponent* newComponent : previewBuilding->GetComponents()) {
				if (newComponent->GetName() == component->GetName()) {
					previewComponent = newComponent;
					break;
				}
			}

			if (!previewComponent) continue;

			TMap<UObjectProperty*, UObject*> componentValuesBeforeCopy;
			for (TFieldIterator<UObjectProperty> PropertyIterator(component->GetClass()); PropertyIterator; ++PropertyIterator) {
				UObjectProperty* property = *PropertyIterator;
				if (property->IsA<UClassProperty>()) continue;
				if (!property->HasAllPropertyFlags(EPropertyFlags::CPF_SaveGame)) {
					void* previewValuePtr = property->ContainerPtrToValuePtr<void>(previewComponent);
					UObject* currentValue = property->GetPropertyValue(previewValuePtr);
					componentValuesBeforeCopy.FindOrAdd(property) = currentValue;
				}
			}
			CopyBySerialization(component, previewComponent);

			for (TFieldIterator<UObjectProperty> PropertyIterator(component->GetClass()); PropertyIterator; ++PropertyIterator) {
				UObjectProperty* property = *PropertyIterator;
				if (property->IsA<UClassProperty>()) continue;

				void* originalValuePtr = property->ContainerPtrToValuePtr<void>(component);
				void* previewValuePtr = property->ContainerPtrToValuePtr<void>(previewComponent);
				
				if (property->HasAllPropertyFlags(EPropertyFlags::CPF_SaveGame)) {
					UObject* original = property->GetPropertyValue(originalValuePtr);
					if (!original) continue;

					FString originalName = original->GetPathName();
					FString newName = original->GetPathName();
					for (int j = 0; j <= i; j++) {
						AFGBuildable* origBuilding = this->mOriginalBuildings[j];
						newName = newName.Replace(*(origBuilding->GetPathName() + "."), *(this->mBuildingsPreview.Find(origBuilding)->mBuildings.Last()->GetPathName() + "."));
					}

					UObject* newObject = FindObject<UObject>(nullptr, *newName);
					property->SetPropertyValue(previewValuePtr, newObject);
				}
				else {
					UObject** previousValuePtr = componentValuesBeforeCopy.Find(property);
					UObject* current = property->GetPropertyValue(previewValuePtr);
					if (previousValuePtr) {
						// only reset references to non static stuff
						if ((!(*previousValuePtr) && current->GetPathName().StartsWith(GetWorld()->GetPathName())) || ((*previousValuePtr) && (*previousValuePtr)->GetPathName().StartsWith(GetWorld()->GetPathName()))) {
							property->SetPropertyValue(previewValuePtr, *previousValuePtr);
						}
					}
				}
			}
		}
		previewBuilding->DeferredBeginPlay();
		for (int j = 0; j < i; j++) {
			AFGBuildable* otherBuildable = this->mOriginalBuildings[j];
			AFGBuildable* otherPreviewBuilding = this->mBuildingsPreview.Find(otherBuildable)->mBuildings.Last();
			for (TFieldIterator<UObjectProperty> PropertyIterator(otherBuildable->GetClass()); PropertyIterator; ++PropertyIterator) {
				UObjectProperty* property = *PropertyIterator;
				if (property->HasAllPropertyFlags(EPropertyFlags::CPF_SaveGame)) {
					void* originalValuePtr = property->ContainerPtrToValuePtr<void>(otherBuildable);
					void* previewValuePtr = property->ContainerPtrToValuePtr<void>(otherPreviewBuilding);
					UObject* original = property->GetPropertyValue(originalValuePtr);

					if (!original) continue;

					FString originalName = original->GetPathName();
					FString newName = original->GetPathName();
					if (!newName.Contains(*(buildable->GetPathName() + "."))) continue;

					newName = newName.Replace(*(buildable->GetPathName() + "."), *(previewBuilding->GetPathName() + "."));

					UObject* newObject = FindObject<UObject>(nullptr, *newName);
					property->SetPropertyValue(previewValuePtr, newObject);
				}
			}
			for (UActorComponent* component : otherBuildable->GetComponents()) {
				UActorComponent* previewComponent = nullptr;
				for (UActorComponent* newComponent : otherPreviewBuilding->GetComponents()) {
					if (newComponent->GetName() == component->GetName()) {
						previewComponent = newComponent;
						break;
					}
				}

				if (!previewComponent) continue;

				for (TFieldIterator<UObjectProperty> PropertyIterator(component->GetClass()); PropertyIterator; ++PropertyIterator) {
					UObjectProperty* property = *PropertyIterator;
					if (property->HasAllPropertyFlags(EPropertyFlags::CPF_SaveGame)) {
						void* originalValuePtr = property->ContainerPtrToValuePtr<void>(component);
						void* previewValuePtr = property->ContainerPtrToValuePtr<void>(previewComponent);
						UObject* original = property->GetPropertyValue(originalValuePtr);

						if (!original) continue;

						FString originalName = original->GetPathName();
						FString newName = original->GetPathName();

						if (!newName.Contains(*(buildable->GetPathName() + "."))) continue;

						newName = newName.Replace(*(buildable->GetPathName() + "."), *(previewBuilding->GetPathName() + "."));

						UObject* newObject = FindObject<UObject>(nullptr, *newName);
						property->SetPropertyValue(previewValuePtr, newObject);
					}
				}
			}
		}
	}
	for (int i = 0; i < this->mOriginalBuildings.Num(); i++) {
		AFGBuildable* buildable = this->mOriginalBuildings[i];
		AFGBuildable* previewBuilding = this->mBuildingsPreview.Find(buildable)->mBuildings.Last();
		for (TFieldIterator<UObjectProperty> PropertyIterator(previewBuilding->GetClass()); PropertyIterator; ++PropertyIterator) {
			UObjectProperty* property = *PropertyIterator;
			if (property->HasAllPropertyFlags(EPropertyFlags::CPF_SaveGame)) {
				void* originalValuePtr = property->ContainerPtrToValuePtr<void>(buildable);
				UObject* originalValue = property->GetPropertyValue(originalValuePtr);
				void* previewValuePtr = property->ContainerPtrToValuePtr<void>(previewBuilding);
				UObject* previewValue = property->GetPropertyValue(previewValuePtr);

				SML::Logging::warning(*buildable->GetName(), ":", *property->GetName(), "=", *originalValue->GetPathName());
				SML::Logging::warning(*previewBuilding->GetName(), ":", *property->GetName(), "=", *previewValue->GetPathName());
			}
		}
		for (UActorComponent* component : buildable->GetComponents()) {
			UActorComponent* previewComponent = nullptr;
			for (UActorComponent* newComponent : previewBuilding->GetComponents()) {
				if (newComponent->GetName() == component->GetName()) {
					previewComponent = newComponent;
					break;
				}
			}
			for (TFieldIterator<UObjectProperty> PropertyIterator(component->GetClass()); PropertyIterator; ++PropertyIterator) {
				UObjectProperty* property = *PropertyIterator;
				if (property->HasAllPropertyFlags(EPropertyFlags::CPF_SaveGame)) {
					void* originalValuePtr = property->ContainerPtrToValuePtr<void>(component);
					UObject* originalValue = property->GetPropertyValue(originalValuePtr);
					void* previewValuePtr = property->ContainerPtrToValuePtr<void>(previewComponent);
					UObject* previewValue = property->GetPropertyValue(previewValuePtr);

					SML::Logging::warning(*buildable->GetName(), ".", *component->GetName(), ":", *property->GetName(), "=", *originalValue->GetPathName());
					SML::Logging::warning(*previewBuilding->GetName(), ".", *previewComponent->GetName(), ":", *property->GetName(), "=", *previewValue->GetPathName());
				}
			}
		}
		if (previewBuilding->IsA<AFGBuildableConveyorBelt>()) {
			SML::Logging::warning("OConnection0 connected: ", ((AFGBuildableConveyorBelt*)buildable)->GetConnection0()->IsConnected());
			SML::Logging::warning("OConnection1 connected: ", ((AFGBuildableConveyorBelt*)buildable)->GetConnection1()->IsConnected());
			SML::Logging::warning("Connection0 connected: ", ((AFGBuildableConveyorBelt*)previewBuilding)->GetConnection0()->IsConnected());
			SML::Logging::warning("Connection1 connected: ", ((AFGBuildableConveyorBelt*)previewBuilding)->GetConnection1()->IsConnected());
		}
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