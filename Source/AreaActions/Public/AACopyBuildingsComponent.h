// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FGBuildable.h"
#include "SaveCustomVersion.h"
// #include "FGBuildableHologram.h"
#include "AACopyBuildingsComponent.generated.h"

USTRUCT()
struct FRotatedBoundingBox
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Center;
	UPROPERTY()
	FVector Extents;
	/** Only has yaw */
	UPROPERTY()
	FRotator Rotation;

	FVector GetCorner(uint32 CornerNum) const;
};

USTRUCT()
struct FCopyPreview
{
	GENERATED_BODY()

	UObject* GetObjectChecked(UObject* Obj, const bool IncludeOriginal = true)
	{
		UObject* Outer = Obj;
		while(Outer && !Objects.Contains(Outer))
			Outer = Outer->GetOuter();
		if(Outer == Obj)
			return Objects[Obj];
		if(Outer)
			return FindObject<UObject>(Objects[Outer], *Obj->GetPathName(Outer));
		if(IncludeOriginal)
			return Obj;
		return nullptr;
	}

	FORCEINLINE UObject* GetObject(UObject* Obj)
	{
		return Objects[Obj];
	}

	FORCEINLINE void AddObject(UObject* Original, UObject* Copy)
	{
		Objects.Add(Original, Copy);
	}

private:
	UPROPERTY()
	TMap<UObject*, UObject*> Objects;
};

// Reimplementation of how I think FSaveCollectorArchive works
// can't wait for modular build
struct FObjectCollector : FArchive
{
	TArray<UObject*>* AllObjects;
	UObject* CurrentObject;
	TArray<UObject*> RootObjects;

	FObjectCollector(TArray<UObject*>* AllObjects)
	{
		SetIsSaving(true);
		ArIsSaveGame = true;
		UsingCustomVersion(FSaveCustomVersion::GUID);
		this->AllObjects = AllObjects;
		this->CurrentObject = nullptr;
	}

	void GetAllObjects(TArray<UObject*>& Root)
	{
		RootObjects = Root;
		for(UObject* Object : Root)
		{
			CurrentObject = Object;
			Object->Serialize(*this);
			this->AllObjects->AddUnique(Object);
		}
	}

private:
	void AddObject(UObject* Object)
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
	
	bool ShouldSave(UObject* Object) const
	{
		if(!CurrentObject) return true;
		if(AActor* Actor = Cast<AActor>(CurrentObject))
		{
			if(Actor->GetOwner() == Object) return false;
		}
		return true;
	}
	
public:
	virtual FArchive& operator<<(UObject*& Value) override
	{
		if(Value && Value->Implements<UFGSaveInterface>() && ShouldSave(Value))
			this->AddObject(Value);
		return *this;
	}
	virtual FArchive& operator<<(FLazyObjectPtr& Value) override
	{
		if(Value.IsValid() && Value.Get() && Value.Get()->Implements<UFGSaveInterface>() && ShouldSave(Value.Get()))
			this->AddObject(Value.Get());
		return *this;
	}
	virtual FArchive& operator<<(FSoftObjectPtr& Value) override
	{
		if(Value.IsValid() && Value.Get() && Value.Get()->Implements<UFGSaveInterface>() && ShouldSave(Value.Get()))
			this->AddObject(Value.Get());
		return *this;
	}
	virtual FArchive& operator<<(FSoftObjectPath& Value) override
	{
		return *this;
	}
	virtual FArchive& operator<<(FWeakObjectPtr& Value) override
	{
		if(Value.IsValid() && Value.Get() && Value.Get()->Implements<UFGSaveInterface>() && ShouldSave(Value.Get()))
			this->AddObject(Value.Get());
		return *this;
	}
};

struct FCopyArchive : FObjectAndNameAsStringProxyArchive
{
	TArray<UObject*> Objects;
	FCopyArchive(FArchive& InnerArchive, TArray<UObject*> InObjects) : FObjectAndNameAsStringProxyArchive(InnerArchive, false)
	{
		this->Objects = InObjects; 
		UsingCustomVersion(FSaveCustomVersion::GUID);
	}

private:
	virtual FArchive& operator<<(UObject*& Obj) override
	{
		if (IsLoading())
		{
			// load the path name to the object
			int32 Idx;
			FString RelativePath;
			InnerArchive << Idx;
			InnerArchive << RelativePath;
			if(Idx == -1)
			{
				// is not copied, reference the same object
				Obj = FindObject<UObject>(nullptr, *RelativePath, false);
			}
			else if(RelativePath == TEXT("None"))
			{
				// It is one of the copied objects
				Obj = Objects[Idx];
			}
			else
			{
				// look up the object by fully qualified pathname
				Obj = FindObject<UObject>(Objects[Idx], *RelativePath, false);
			}
		}
		else
		{
			int32 Idx = -1;
			UObject* Outer = nullptr;
			if(Obj)
			{
				Outer = Obj;
				while(Outer && !Objects.Contains(Outer))
					Outer = Outer->GetOuter();
				Idx = Objects.Find(Outer);
			}
			FString SavedString(Obj->GetPathName(Outer));
			InnerArchive << Idx;
			InnerArchive << SavedString;
		}
		return *this;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class AREAACTIONS_API UAACopyBuildingsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAACopyBuildingsComponent();

	bool SetActors(TArray<AActor*>& Actors, TArray<AFGBuildable*>& OutBuildingsWithIssues);
	bool SetBuildings(TArray<AFGBuildable*>& Buildings, TArray<AFGBuildable*>& OutBuildingsWithIssues);
	bool ValidateObjects(TArray<AFGBuildable*>& OutBuildingsWithIssues);

	FORCEINLINE FVector GetBuildingsCenter() const { return BuildingsBounds.Center; }
	FORCEINLINE FRotatedBoundingBox GetBounds() const { return BuildingsBounds; }

	int AddCopy(FVector Offset, FRotator Rotation, FVector RotationCenter, bool Relative = true);
	FORCEINLINE int AddCopy(const FVector Offset, const FRotator Rotation, const bool Relative = true) { return this->AddCopy(Offset, Rotation, GetBuildingsCenter(), Relative); }
	void MoveCopy(int CopyId, FVector Offset, FRotator Rotation, FVector RotationCenter, bool Relative = true);
	FORCEINLINE void MoveCopy(const int CopyId, const FVector Offset, const FRotator Rotation, const bool Relative = true) { this->MoveCopy(CopyId, Offset, Rotation, GetBuildingsCenter(), Relative); }
	void RemoveCopy(int CopyId);
	
	void Finish();

private:
	void FixReferencesForCopy(int CopyId);
	bool ValidateObject(UObject* Object);

	void CalculateBounds();
private:
	int32 CurrentId;
	
	UPROPERTY()
	TArray<UObject*> Original;

	TArray<TPair<FVector, FRotator>> CopyLocations;

	UPROPERTY()
	TMap<int32, FCopyPreview> Preview;

	TSet<UProperty*> ValidCheckSkipProperties;

	FRotatedBoundingBox BuildingsBounds;
};
