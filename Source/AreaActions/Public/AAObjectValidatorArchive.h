// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Archive.h"
#include "FGSaveInterface.h"
#include "UObject/Object.h"

struct FAAObjectValidatorArchive : FArchive
{
	FAAObjectValidatorArchive(TArray<UObject*>& InAllObjects);
	
	virtual FArchive& operator<<(UObject*& Value) override;
	virtual FArchive& operator<<(FLazyObjectPtr& Value) override;
	virtual FArchive& operator<<(FSoftObjectPtr& Value) override;
	virtual FArchive& operator<<(FSoftObjectPath& Value) override;
	virtual FArchive& operator<<(FWeakObjectPtr& Value) override;

	bool Validate(UObject* Object);
	
protected:
	bool IsValid(UObject* Object) const;
	void SetInvalid(UObject* InvalidObject);

private:
	TArray<UObject*>& AllObjects;
	UObject* CurrentObject;
	bool Valid;
};
