// Fill out your copyright notice in the Description page of Project Settings.


#include "AAEquipment.h"
#include "SML/util/Logging.h"
#include "FGHUD.h"
#include "FGBuildable.h"
#include "UI/FGGameUI.h"

AAAEquipment::AAAEquipment() : Super() {
	this->mSelectionMode = SM_CORNER;
	this->mAreaMinZ = MinZ;
	this->mAreaMaxZ = MaxZ;
}

void AAAEquipment::BeginPlay() {
	this->mBottomIndicator = GetWorld()->SpawnActor<AAAHeightIndicator>(HeightIndicatorClass, FVector(0, 0, this->mAreaMinZ), FRotator::ZeroRotator);
	this->mBottomIndicator->SetIndicatorType(HIT_BOTTOM);
	this->mTopIndicator = GetWorld()->SpawnActor<AAAHeightIndicator>(HeightIndicatorClass, FVector(0, 0, this->mAreaMaxZ), FRotator::ZeroRotator);
	this->mTopIndicator->SetIndicatorType(HIT_TOP);
	this->UpdateHeight();
}

void AAAEquipment::Equip(class AFGCharacterPlayer* character) {
	Super::Equip(character);
	this->UpdateExtraActors();
}

void AAAEquipment::PrimaryFire() {
	FHitResult hitResult;
	switch (this->mSelectionMode) {
	case SM_CORNER:
		if (RaycastMouseWithRange(hitResult, false, true, true)) {
			if (hitResult.Actor->IsA<AAACornerIndicator>()) {
				AAACornerIndicator* hitCorner = (AAACornerIndicator*)hitResult.Actor.Get();
				int cornerIdx = mCornerIndicators.Find(hitCorner);
				this->RemoveCorner(cornerIdx);
			}
			else {
				this->AddCorner(hitResult.Location);
			}
		}
		break;
	case SM_BOTTOM:
		if (RaycastMouseWithRange(hitResult, false, true, false)) {
			if (hitResult.Actor == this->mBottomIndicator) {
				this->mAreaMinZ = MinZ;
			}
			else {
				this->mAreaMinZ = hitResult.Location.Z;
				if (this->mAreaMaxZ < this->mAreaMinZ) {
					float tmp = this->mAreaMinZ;
					this->mAreaMinZ = this->mAreaMaxZ;
					this->mAreaMaxZ = tmp;
				}
			}
			this->UpdateHeight();
		}
		break;
	case SM_TOP:
		if (RaycastMouseWithRange(hitResult, false, true, false)) {
			if (hitResult.Actor == this->mTopIndicator) {
				this->mAreaMaxZ = MaxZ;
			}
			else {
				this->mAreaMaxZ = hitResult.Location.Z;
				if (this->mAreaMaxZ < this->mAreaMinZ) {
					float tmp = this->mAreaMinZ;
					this->mAreaMinZ = this->mAreaMaxZ;
					this->mAreaMaxZ = tmp;
				}
			}
			this->UpdateHeight();
		}
		break;
	case SM_BUILDING:
		if (RaycastMouseWithRange(hitResult, true, true, true)) {
			if (hitResult.Actor->IsA<AFGBuildable>()) {
				if (this->mExtraActors.Contains(hitResult.Actor.Get())) {
					this->mExtraActors.Remove(hitResult.Actor.Get());
				}
				else {
					this->mExtraActors.Add(hitResult.Actor.Get());
				}
				this->UpdateExtraActors();
			}
		}
		break;
	default:
		break;
	}
}

void AAAEquipment::SecondaryFire() {
}

void AAAEquipment::SelectMap() {
	this->ClearSelection();
	this->AddCorner(FVector(-350000.0, -350000.0, 0));
	this->AddCorner(FVector(450000.0, -350000.0, 0));
	this->AddCorner(FVector(450000.0, 450000.0, 0));
	this->AddCorner(FVector(-350000.0, 450000.0, 0));
	this->UpdateHeight();
}

void AAAEquipment::ClearSelection() {
	for (int i = this->mAreaCorners.Num() - 1; i >= 0; i--) {
		this->RemoveCorner(i);
	}
	this->mAreaMinZ = MinZ;
	this->mAreaMaxZ = MaxZ;
	this->UpdateHeight();
}

void AAAEquipment::UpdateHeight() {
	for (AAACornerIndicator* indicator : this->mCornerIndicators) {
		indicator->UpdateHeight(this->mAreaMinZ, this->mAreaMaxZ);
	}
	for (AAAWallIndicator* indicator : this->mWallIndicators) {
		indicator->UpdateHeight(this->mAreaMinZ, this->mAreaMaxZ);
	}
	this->mBottomIndicator->UpdateHeight(this->mAreaMinZ, this->mAreaMaxZ);
	this->mBottomIndicator->SetActorHiddenInGame(this->mAreaMinZ == MinZ);
	this->mTopIndicator->UpdateHeight(this->mAreaMinZ, this->mAreaMaxZ);
	this->mTopIndicator->SetActorHiddenInGame(this->mAreaMaxZ == MaxZ);
}

AAAWallIndicator* AAAEquipment::CreateWallIndicator(FVector from, FVector to) {
	FVector middle = (from + to) / 2;
	float length = FVector::Dist(from, to);
	float rotation = (to - from).Rotation().Yaw;

	AAAWallIndicator* indicator = GetWorld()->SpawnActor<AAAWallIndicator>(WallIndicatorClass, middle, FRotator(0, rotation, 0));
	indicator->SetActorScale3D(FVector(length, 1, 1));
	indicator->UpdateHeight(this->mAreaMinZ, this->mAreaMaxZ);
	return indicator;
}

AAACornerIndicator* AAAEquipment::CreateCornerIndicator(FVector location) {
	AAACornerIndicator* indicator = GetWorld()->SpawnActor<AAACornerIndicator>(CornerIndicatorClass, location, FRotator::ZeroRotator);
	indicator->UpdateHeight(this->mAreaMinZ, this->mAreaMaxZ);
	return indicator;
}

void AAAEquipment::AddCorner(FVector location) {
	if(this->mWallIndicators.Num() > 1) {
		AAAWallIndicator* wall2 = this->mWallIndicators[this->mWallIndicators.Num() - 1];
		this->mWallIndicators.RemoveAt(this->mWallIndicators.Num() - 1);
		wall2->Destroy();
	}

	this->mCornerIndicators.Add(CreateCornerIndicator(location));

	if (this->mAreaCorners.Num() > 0) {
		this->mWallIndicators.Add(CreateWallIndicator(this->mAreaCorners[this->mAreaCorners.Num() - 1], location));
	}
	if(this->mAreaCorners.Num() > 1) {
		this->mWallIndicators.Add(CreateWallIndicator(location, this->mAreaCorners[0]));
	}

	this->mAreaCorners.Add(location);
}

void AAAEquipment::RemoveCorner(int cornerIdx) {
	AAACornerIndicator* corner = this->mCornerIndicators[cornerIdx];
	this->mCornerIndicators.RemoveAt(cornerIdx);
	corner->Destroy();
	this->mAreaCorners.RemoveAt(cornerIdx);
	
	if (this->mWallIndicators.Num() > 0) {
		AAAWallIndicator* wall = this->mWallIndicators[cornerIdx % this->mWallIndicators.Num()];
		this->mWallIndicators.RemoveAt(cornerIdx % this->mWallIndicators.Num());
		wall->Destroy();
	}
	if (this->mWallIndicators.Num() > 0) {
		AAAWallIndicator* wall = this->mWallIndicators[(cornerIdx - 1 + this->mWallIndicators.Num()) % this->mWallIndicators.Num()];
		this->mWallIndicators.RemoveAt((cornerIdx - 1 + this->mWallIndicators.Num()) % this->mWallIndicators.Num());
		wall->Destroy();
	}
	if (this->mAreaCorners.Num() > 2) {
		this->mWallIndicators.Insert(CreateWallIndicator(
				this->mAreaCorners[(cornerIdx - 1 + this->mAreaCorners.Num()) % this->mAreaCorners.Num()],
				this->mAreaCorners[cornerIdx % this->mAreaCorners.Num()]),
			(cornerIdx - 1 + this->mAreaCorners.Num()) % this->mAreaCorners.Num());
	}
}

bool AAAEquipment::RaycastMouseWithRange(FHitResult& out_hitResult, bool ignoreCornerIndicators, bool ignoreWallIndicators, bool ignoreHeightIndicators, TArray<AActor*> otherIgnoredActors) {
	TArray<AActor*> ignoredActors;

	if (ignoreCornerIndicators) {
		ignoredActors.Append(this->mCornerIndicators);
	}
	if (ignoreWallIndicators) {
		ignoredActors.Append(this->mWallIndicators);
	}
	if (ignoreHeightIndicators) {
		ignoredActors.Add(this->mTopIndicator);
		ignoredActors.Add(this->mBottomIndicator);
	}

	APlayerCameraManager* cameraManager = ((AFGPlayerController*)GetInstigatorCharacter()->GetController())->PlayerCameraManager;

	FVector cameraLocation = cameraManager->GetCameraLocation();
	FVector cameraDirection = cameraManager->GetActorForwardVector();

	FVector lineTraceEnd = cameraLocation + cameraDirection * MaxRaycastDistance;

	FCollisionQueryParams params = FCollisionQueryParams::DefaultQueryParam;
	params.AddIgnoredActors(ignoredActors);
	params.AddIgnoredActors(otherIgnoredActors);
	return GetWorld()->LineTraceSingleByChannel(out_hitResult, cameraLocation, lineTraceEnd, ECollisionChannel::ECC_Visibility, params);
}

void AAAEquipment::UpdateExtraActors() {
	UFGOutlineComponent::Get(GetWorld())->ShowDismantlePendingMaterial(this->mExtraActors);
}

void AAAEquipment::RunAction(TSubclassOf<AAAAction> actionClass) {
	SML::Logging::warning(*actionClass->GetPathName());
}