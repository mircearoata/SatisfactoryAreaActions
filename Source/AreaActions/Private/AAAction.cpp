// Fill out your copyright notice in the Description page of Project Settings.

#include "AAAction.h"
#include "AAEquipment.h"
#include "SML/util/Logging.h"

void AAAAction::Init_Implementation() {
}

void AAAAction::Run() {
	this->InternalRun();
	this->PostRun();
}

void AAAAction::InternalRun() {
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