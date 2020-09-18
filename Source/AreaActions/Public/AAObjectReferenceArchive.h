﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ObjectAndNameAsStringProxyArchive.h"
#include "UObject/Object.h"

struct FAAObjectReferenceArchive : FObjectAndNameAsStringProxyArchive
{
	TArray<UObject*> Objects;
	FAAObjectReferenceArchive(FArchive& InnerArchive, TArray<UObject*> InObjects);
	
	virtual FArchive& operator<<(UObject*& Obj) override;
};
