// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Serialization/Archive.h"
#include "UObject/Object.h"

struct FAAObjectCollectorArchive : FArchive
{
	FAAObjectCollectorArchive(TArray<UObject*>* AllObjects);
	
	virtual FArchive& operator<<(UObject*& Value) override;
	virtual FArchive& operator<<(FLazyObjectPtr& Value) override;
	virtual FArchive& operator<<(FSoftObjectPtr& Value) override;
	virtual FArchive& operator<<(FSoftObjectPath& Value) override;
	virtual FArchive& operator<<(FWeakObjectPtr& Value) override;

	void GetAllObjects(TArray<UObject*>& Root);

protected:
	void AddObject(UObject* Object);	
	bool ShouldSave(UObject* Object) const;

private:
	TArray<UObject*>* AllObjects;
	UObject* CurrentObject;
	TArray<UObject*> RootObjects;
};
