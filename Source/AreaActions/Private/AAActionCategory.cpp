// Fill out your copyright notice in the Description page of Project Settings.


#include "AAActionCategory.h"

#include "SubclassOf.h"

FText UAAActionCategory::GetCategoryName(const TSubclassOf<UAAActionCategory> InClass)
{
    if(!InClass) return FText::FromString(TEXT("N/A"));
    return InClass.GetDefaultObject()->Name;
}

FSlateBrush UAAActionCategory::GetCategoryIcon(const TSubclassOf<UAAActionCategory> InClass)
{
    if(!InClass) return FSlateBrush();
    return InClass.GetDefaultObject()->Icon;
}
