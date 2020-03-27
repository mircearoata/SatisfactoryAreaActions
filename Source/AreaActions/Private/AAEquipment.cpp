// Fill out your copyright notice in the Description page of Project Settings.

#include "AAEquipment.h"
#include "SML/util/Logging.h"
#include "FGHUD.h"
#include "FGBuildable.h"
#include "UI/FGGameUI.h"
#include "GenericPlatform/GenericPlatformMath.h"

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
	this->DelayedUpdateExtraActors();
}

void AAAEquipment::UnEquip() {
	Super::UnEquip();
	if (this->mExtraActors.Num() > 0) {
		UFGOutlineComponent::Get(GetWorld())->HideAllDismantlePendingMaterial();
	}
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
				this->AddCorner(FVector2D(hitResult.Location.X, hitResult.Location.Y));
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
	this->AddCorner(FVector2D(-350000.0, -350000.0));
	this->AddCorner(FVector2D(450000.0, -350000.0));
	this->AddCorner(FVector2D(450000.0, 450000.0));
	this->AddCorner(FVector2D(-350000.0, 450000.0));
	this->UpdateHeight();
}

void AAAEquipment::ClearSelection() {
	for (int i = this->mAreaCorners.Num() - 1; i >= 0; i--) {
		this->RemoveCorner(i);
	}
	this->mAreaMinZ = MinZ;
	this->mAreaMaxZ = MaxZ;
	this->UpdateHeight();
	this->mExtraActors.Empty();
	this->UpdateExtraActors();
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

AAAWallIndicator* AAAEquipment::CreateWallIndicator(FVector2D from, FVector2D to) {
	FVector2D middle = (from + to) / 2;
	float length = FVector::Dist(FVector(from, 0), FVector(to, 0));
	float rotation = FVector((to - from), 0).Rotation().Yaw;

	AAAWallIndicator* indicator = GetWorld()->SpawnActor<AAAWallIndicator>(WallIndicatorClass, FVector(middle, 0), FRotator(0, rotation, 0));
	indicator->SetActorScale3D(FVector(length, 1, 1));
	indicator->UpdateHeight(this->mAreaMinZ, this->mAreaMaxZ);
	return indicator;
}

AAACornerIndicator* AAAEquipment::CreateCornerIndicator(FVector2D location) {
	AAACornerIndicator* indicator = GetWorld()->SpawnActor<AAACornerIndicator>(CornerIndicatorClass, FVector(location, 0), FRotator::ZeroRotator);
	indicator->UpdateHeight(this->mAreaMinZ, this->mAreaMaxZ);
	return indicator;
}

void AAAEquipment::AddCorner(FVector2D location) {
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

void GetMiddleOfActors(TArray<AActor*>& actors, FVector& middle) {
	if (actors.Num() == 0)
		return;
	FVector min;
	FVector max;
	FVector tmp;

	actors[0]->GetActorBounds(true, min, tmp);
	max = min;

	FVector actorCenter;
	for (int i = 1; i < actors.Num(); i++) {
		actors[i]->GetActorBounds(true, actorCenter, tmp);
		min = min.ComponentMin(actorCenter);
		max = max.ComponentMax(actorCenter);
	}

	middle = (min + max) / 2;
}

void GetMostCommonRotation(TArray<AActor*>& actors, FRotator& rotation) {
	TMap<float, int> rotationCount;
	for (int i = 0; i < actors.Num(); i++) {
		float actorRotationMod = FGenericPlatformMath::Fmod(FGenericPlatformMath::Fmod(actors[i]->GetActorRotation().Yaw, 90) + 90, 90);
		rotationCount.FindOrAdd(actorRotationMod)++;
	}

	float bestRotation = 0;
	int maxCount = 0;
	for (auto& rotationCnt : rotationCount) {
		if (rotationCnt.Value > maxCount) {
			maxCount = rotationCnt.Value;
			bestRotation = rotationCnt.Key;
		}
	}
	rotation = FRotator(0, bestRotation, 0);
}

void AAAEquipment::RunAction(TSubclassOf<AAAAction> actionClass) {
	if (this->mAreaCorners.Num() < 3 && this->mExtraActors.Num() == 0) {
		this->ShowMessageOk(WidgetTitle, AreaNotSetMessage);
		return;
	}
	if (this->mCurrentAction) {
		this->ShowMessageOk(WidgetTitle, ConflictingActionRunningMessage);
		return;
	}
	TArray<AActor*> actorsInArea;
	this->GetAllActorsInArea(actorsInArea);
	actorsInArea.Append(this->mExtraActors);
	FVector middle;
	GetMiddleOfActors(actorsInArea, middle);
	FRotator rotation;
	GetMostCommonRotation(actorsInArea, rotation);

	this->mCurrentAction = GetWorld()->SpawnActor<AAAAction>(actionClass, middle, rotation);
	this->mCurrentAction->SetAAEquipment(this);
	this->mCurrentAction->SetActors(actorsInArea);
	this->mCurrentAction->Init();
}

void AAAEquipment::ActionDone() {
	this->mCurrentAction->Destroy();
	this->mCurrentAction = nullptr;
}

bool IsPointInPolgon(FVector2D point, TArray<FVector2D>& polygon) {
	if (polygon.Num() < 2)
		return false;
	FVector2D min, max;
	min = max = polygon[0];
	for (int i = 1; i < polygon.Num(); i++)
	{
		min = FVector2D::Min(min, polygon[i]);
		max = FVector2D::Max(max, polygon[i]);
	}

	if (point.X < min.X || point.X > max.X || point.Y < min.Y || point.Y > max.Y)
		return false;

	bool inside = false;
	for (int i = 0, j = polygon.Num() - 1; i < polygon.Num(); j = i++)
	{
		if ((polygon[i].Y > point.Y) != (polygon[j].Y > point.Y) &&
			point.X < (polygon[j].X - polygon[i].X) * (point.Y - polygon[i].Y) / (polygon[j].Y - polygon[i].Y) + polygon[i].X)
		{
			inside = !inside;
		}
	}

	return inside;
}

bool IsActorInArea(AActor* actor, TArray<FVector2D>& corners, float minZ, float maxZ) {
	return minZ <= actor->GetActorLocation().Z && actor->GetActorLocation().Z <= maxZ && IsPointInPolgon(FVector2D(actor->GetActorLocation().X, actor->GetActorLocation().Y), corners);
}

void AAAEquipment::GetAllActorsInArea(TArray<AActor*>& out_actors) {
	for (TActorIterator<AFGBuildable> ActorIt(GetWorld()); ActorIt; ++ActorIt) {
		if (IsActorInArea(*ActorIt, this->mAreaCorners, this->mAreaMinZ, this->mAreaMaxZ)) {
			out_actors.Add(*ActorIt);
		}
	}
}

AFGPlayerController* AAAEquipment::GetOwningController() {
	return (AFGPlayerController*)this->GetInstigatorCharacter()->GetController();
}