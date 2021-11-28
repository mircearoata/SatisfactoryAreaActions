// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "UObject/Object.h"

struct FAAObjectReferenceArchive : FObjectAndNameAsStringProxyArchive
{
	TArray<UObject*> Objects;
	FAAObjectReferenceArchive(FArchive& InnerArchive, TArray<UObject*> InObjects);

	void GetReferenceData(UObject* Obj, int32& Idx, FString& RelativePath) const;
	UObject* ResolveReference(const int32& Idx, const FString& RelativePath);
	virtual FArchive& operator<<(UObject*& Obj) override;
};
