// Fill out your copyright notice in the Description page of Project Settings.

#include "AAAction.h"
#include "AAEquipment.h"

void AAAAction::Run_Implementation() {
}

void AAAAction::Done()
{
	this->AAEquipment->ActionDone();
}

void AAAAction::Cancel()
{
	this->OnCancel();
	this->Done();
}

FText AAAAction::GetActionName(const TSubclassOf<AAAAction> InClass)
{
	if(!InClass) return FText::FromString(TEXT("N/A"));
	return InClass.GetDefaultObject()->Name;
}

FText AAAAction::GetActionDescription(const TSubclassOf<AAAAction> InClass)
{
	if(!InClass) return FText::FromString(TEXT("N/A"));
	return InClass.GetDefaultObject()->Description;
}

TSubclassOf<UAAActionCategory> AAAAction::GetActionCategory(const TSubclassOf<AAAAction> InClass)
{
	if(!InClass) return UAAActionCategory::StaticClass();
	return InClass.GetDefaultObject()->Category;
}

FSlateBrush AAAAction::GetActionIcon(const TSubclassOf<AAAAction> InClass)
{
	if(!InClass) return FSlateBrush();
	return InClass.GetDefaultObject()->Icon;
}

void AAAAction::PrimaryFire_Implementation()
{
}

void AAAAction::MouseWheelUp_Implementation()
{
}

void AAAAction::SecondaryFire_Implementation()
{
}

void AAAAction::OnCancel_Implementation()
{
}

void AAAAction::MouseWheelDown_Implementation()
{
}