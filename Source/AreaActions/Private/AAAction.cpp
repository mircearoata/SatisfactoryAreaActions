// Fill out your copyright notice in the Description page of Project Settings.

#include "AAAction.h"
#include "AAEquipment.h"
#include "SML/util/Logging.h"

void AAAAction::Run_Implementation() {
}

void AAAAction::InternalRun() {
	this->Run();
	this->PostRun();
}

void AAAAction::Done() {
	this->mAAEquipment->ActionDone();
}

void AAAAction::PostRun_Implementation() {
	if (CloseAfterRun) {
		this->Done();
	}
}

void AAAAction::PrimaryFire_Implementation() {
}

void AAAAction::SecondaryFire_Implementation() {
}