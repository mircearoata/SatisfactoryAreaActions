// Fill out your copyright notice in the Description page of Project Settings.

#include "AAEquipment.h"
#include "SML/util/Logging.h"
#include "FGHUD.h"
#include "FGBuildable.h"
#include "UI/FGGameUI.h"
#include "GenericPlatform/GenericPlatformMath.h"

AAAEquipment::AAAEquipment() : Super() {
	this->SelectionMode = SM_Corner;
	this->AreaMinZ = MinZ;
	this->AreaMaxZ = MaxZ;
}

void AAAEquipment::BeginPlay() {
	this->BottomIndicator = GetWorld()->SpawnActor<AAAHeightIndicator>(HeightIndicatorClass, FVector(0, 0, this->AreaMinZ), FRotator::ZeroRotator);
	this->BottomIndicator->SetIndicatorType(Bottom);
	this->TopIndicator = GetWorld()->SpawnActor<AAAHeightIndicator>(HeightIndicatorClass, FVector(0, 0, this->AreaMaxZ), FRotator::ZeroRotator);
	this->TopIndicator->SetIndicatorType(Top);
	this->UpdateHeight();
}

void AAAEquipment::Equip(class AFGCharacterPlayer* Character) {
	Super::Equip(Character);
	this->DelayedUpdateExtraActors();
}

void AAAEquipment::UnEquip() {
	Super::UnEquip();
	if (this->ExtraActors.Num() > 0) {
		UFGOutlineComponent::Get(GetWorld())->HideAllDismantlePendingMaterial();
	}
}

void AAAEquipment::PrimaryFire() {
	FHitResult HitResult;
	switch (this->SelectionMode) {
	case SM_Corner:
		if (RaycastMouseWithRange(HitResult, false, true, true)) {
			if (HitResult.Actor->IsA<AAACornerIndicator>()) {
				AAACornerIndicator* HitCorner = static_cast<AAACornerIndicator*>(HitResult.Actor.Get());
				const int CornerIdx = CornerIndicators.Find(HitCorner);
				this->RemoveCorner(CornerIdx);
			}
			else {
				this->AddCorner(FVector2D(HitResult.Location.X, HitResult.Location.Y));
			}
		}
		break;
	case SM_Bottom:
		if (RaycastMouseWithRange(HitResult, false, true, false)) {
			if (HitResult.Actor == this->BottomIndicator) {
				this->AreaMinZ = MinZ;
			}
			else {
				this->AreaMinZ = HitResult.Location.Z;
				if (this->AreaMaxZ < this->AreaMinZ) {
					const float Tmp = this->AreaMinZ;
					this->AreaMinZ = this->AreaMaxZ;
					this->AreaMaxZ = Tmp;
				}
			}
			this->UpdateHeight();
		}
		break;
	case SM_Top:
		if (RaycastMouseWithRange(HitResult, false, true, false)) {
			if (HitResult.Actor == this->TopIndicator) {
				this->AreaMaxZ = MaxZ;
			}
			else {
				this->AreaMaxZ = HitResult.Location.Z;
				if (this->AreaMaxZ < this->AreaMinZ) {
					const float Tmp = this->AreaMinZ;
					this->AreaMinZ = this->AreaMaxZ;
					this->AreaMaxZ = Tmp;
				}
			}
			this->UpdateHeight();
		}
		break;
	case SM_Building:
		if (RaycastMouseWithRange(HitResult, true, true, true)) {
			if (HitResult.Actor->IsA<AFGBuildable>()) {
				if (this->ExtraActors.Contains(HitResult.Actor.Get())) {
					this->ExtraActors.Remove(HitResult.Actor.Get());
				}
				else {
					this->ExtraActors.Add(HitResult.Actor.Get());
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
	for (int i = this->AreaCorners.Num() - 1; i >= 0; i--) {
		this->RemoveCorner(i);
	}
	this->AreaMinZ = MinZ;
	this->AreaMaxZ = MaxZ;
	this->UpdateHeight();
	this->ExtraActors.Empty();
	this->UpdateExtraActors();
}

void AAAEquipment::UpdateHeight() {
	for (AAACornerIndicator* Indicator : this->CornerIndicators) {
		Indicator->UpdateHeight(this->AreaMinZ, this->AreaMaxZ);
	}
	for (AAAWallIndicator* Indicator : this->WallIndicators) {
		Indicator->UpdateHeight(this->AreaMinZ, this->AreaMaxZ);
	}
	this->BottomIndicator->UpdateHeight(this->AreaMinZ, this->AreaMaxZ);
	this->BottomIndicator->SetActorHiddenInGame(this->AreaMinZ == MinZ);
	this->TopIndicator->UpdateHeight(this->AreaMinZ, this->AreaMaxZ);
	this->TopIndicator->SetActorHiddenInGame(this->AreaMaxZ == MaxZ);
}

AAAWallIndicator* AAAEquipment::CreateWallIndicator(const FVector2D From, const FVector2D To) const
{
	const FVector2D Middle = (From + To) / 2;
	const float Length = FVector::Dist(FVector(From, 0), FVector(To, 0));
	const float Rotation = FVector((To - From), 0).Rotation().Yaw;

	AAAWallIndicator* Indicator = GetWorld()->SpawnActor<AAAWallIndicator>(WallIndicatorClass, FVector(Middle, 0), FRotator(0, Rotation, 0));
	Indicator->SetActorScale3D(FVector(Length, 1, 1));
	Indicator->UpdateHeight(this->AreaMinZ, this->AreaMaxZ);
	return Indicator;
}

AAACornerIndicator* AAAEquipment::CreateCornerIndicator(const FVector2D Location) const
{
	AAACornerIndicator* Indicator = GetWorld()->SpawnActor<AAACornerIndicator>(CornerIndicatorClass, FVector(Location, 0), FRotator::ZeroRotator);
	Indicator->UpdateHeight(this->AreaMinZ, this->AreaMaxZ);
	return Indicator;
}

void AAAEquipment::AddCorner(const FVector2D Location) {
	if(this->WallIndicators.Num() > 1) {
		AAAWallIndicator* Wall = this->WallIndicators[this->WallIndicators.Num() - 1];
		this->WallIndicators.RemoveAt(this->WallIndicators.Num() - 1);
		Wall->Destroy();
	}

	this->CornerIndicators.Add(CreateCornerIndicator(Location));

	if (this->AreaCorners.Num() > 0) {
		this->WallIndicators.Add(CreateWallIndicator(this->AreaCorners[this->AreaCorners.Num() - 1], Location));
	}
	if(this->AreaCorners.Num() > 1) {
		this->WallIndicators.Add(CreateWallIndicator(Location, this->AreaCorners[0]));
	}

	this->AreaCorners.Add(Location);
}

void AAAEquipment::RemoveCorner(const int CornerIdx) {
	AAACornerIndicator* Corner = this->CornerIndicators[CornerIdx];
	this->CornerIndicators.RemoveAt(CornerIdx);
	Corner->Destroy();
	this->AreaCorners.RemoveAt(CornerIdx);
	
	if (this->WallIndicators.Num() > 0) {
		AAAWallIndicator* Wall = this->WallIndicators[CornerIdx % this->WallIndicators.Num()];
		this->WallIndicators.RemoveAt(CornerIdx % this->WallIndicators.Num());
		Wall->Destroy();
	}
	if (this->WallIndicators.Num() > 0) {
		AAAWallIndicator* Wall = this->WallIndicators[(CornerIdx - 1 + this->WallIndicators.Num()) % this->WallIndicators.Num()];
		this->WallIndicators.RemoveAt((CornerIdx - 1 + this->WallIndicators.Num()) % this->WallIndicators.Num());
		Wall->Destroy();
	}
	if (this->AreaCorners.Num() > 2) {
		this->WallIndicators.Insert(CreateWallIndicator(
				this->AreaCorners[(CornerIdx - 1 + this->AreaCorners.Num()) % this->AreaCorners.Num()],
				this->AreaCorners[CornerIdx % this->AreaCorners.Num()]),
			(CornerIdx - 1 + this->AreaCorners.Num()) % this->AreaCorners.Num());
	}
}

bool AAAEquipment::RaycastMouseWithRange(FHitResult& OutHitResult, const bool bIgnoreCornerIndicators, const bool bIgnoreWallIndicators, const bool bIgnoreHeightIndicators, const TArray<AActor*> OtherIgnoredActors) const
{
	TArray<AActor*> IgnoredActors;

	if (bIgnoreCornerIndicators) {
		IgnoredActors.Append(this->CornerIndicators);
	}
	if (bIgnoreWallIndicators) {
		IgnoredActors.Append(this->WallIndicators);
	}
	if (bIgnoreHeightIndicators) {
		IgnoredActors.Add(this->TopIndicator);
		IgnoredActors.Add(this->BottomIndicator);
	}

	APlayerCameraManager* CameraManager = static_cast<AFGPlayerController*>(GetInstigatorCharacter()->GetController())->PlayerCameraManager;

	const FVector CameraLocation = CameraManager->GetCameraLocation();
	const FVector CameraDirection = CameraManager->GetActorForwardVector();

	const FVector LineTraceEnd = CameraLocation + CameraDirection * MaxRaycastDistance;

	FCollisionQueryParams Params = FCollisionQueryParams::DefaultQueryParam;
	Params.AddIgnoredActors(IgnoredActors);
	Params.AddIgnoredActors(OtherIgnoredActors);
	return GetWorld()->LineTraceSingleByChannel(OutHitResult, CameraLocation, LineTraceEnd, ECC_Visibility, Params);
}

void AAAEquipment::UpdateExtraActors() const
{
	UFGOutlineComponent::Get(GetWorld())->ShowDismantlePendingMaterial(this->ExtraActors);
}

void GetMiddleOfActors(TArray<AActor*>& Actors, FVector& Middle) {
	if (Actors.Num() == 0)
		return;
	FVector Min;
	FVector Tmp;

	Actors[0]->GetActorBounds(true, Min, Tmp);
	FVector Max = Min;

	FVector ActorCenter;
	for (int i = 1; i < Actors.Num(); i++) {
		Actors[i]->GetActorBounds(true, ActorCenter, Tmp);
		Min = Min.ComponentMin(ActorCenter);
		Max = Max.ComponentMax(ActorCenter);
	}

	Middle = (Min + Max) / 2;
}

void GetMostCommonRotation(TArray<AActor*>& Actors, FRotator& Rotation) {
	TMap<float, int> RotationCount;
	for (int i = 0; i < Actors.Num(); i++)
		RotationCount.FindOrAdd(FGenericPlatformMath::Fmod(FGenericPlatformMath::Fmod(Actors[i]->GetActorRotation().Yaw, 90) + 90, 90))++;

	float BestRotation = 0;
	int MaxCount = 0;
	for (auto& RotationCnt : RotationCount) {
		if (RotationCnt.Value > MaxCount) {
			MaxCount = RotationCnt.Value;
			BestRotation = RotationCnt.Key;
		}
	}
	Rotation = FRotator(0, BestRotation, 0);
}

void AAAEquipment::RunAction(const TSubclassOf<AAAAction> ActionClass) {
	if (this->AreaCorners.Num() < 3 && this->ExtraActors.Num() == 0) {
		this->ShowMessageOk(WidgetTitle, AreaNotSetMessage);
		return;
	}
	if (this->CurrentAction) {
		this->ShowMessageOk(WidgetTitle, ConflictingActionRunningMessage);
		return;
	}
	TArray<AActor*> ActorsInArea;
	this->GetAllActorsInArea(ActorsInArea);
	ActorsInArea.Append(this->ExtraActors);
	FVector Middle;
	GetMiddleOfActors(ActorsInArea, Middle);
	FRotator Rotation;
	GetMostCommonRotation(ActorsInArea, Rotation);

	this->CurrentAction = GetWorld()->SpawnActor<AAAAction>(ActionClass, Middle, Rotation);
	this->CurrentAction->SetAAEquipment(this);
	this->CurrentAction->SetActors(ActorsInArea);
	this->CurrentAction->InternalRun();
}

void AAAEquipment::ActionDone() {
	this->CurrentAction->Destroy();
	this->CurrentAction = nullptr;
}

bool IsPointInPolygon(const FVector2D Point, TArray<FVector2D>& Polygon) {
	if (Polygon.Num() < 2)
		return false;
	FVector2D Min = Polygon[0];
	FVector2D Max = Min;
	for (int i = 1; i < Polygon.Num(); i++)
	{
		Min = FVector2D::Min(Min, Polygon[i]);
		Max = FVector2D::Max(Max, Polygon[i]);
	}

	if (Point.X < Min.X || Point.X > Max.X || Point.Y < Min.Y || Point.Y > Max.Y)
		return false;

	bool bInside = false;
	for (int i = 0, j = Polygon.Num() - 1; i < Polygon.Num(); j = i++)
	{
		if ((Polygon[i].Y > Point.Y) != (Polygon[j].Y > Point.Y) &&
			Point.X < (Polygon[j].X - Polygon[i].X) * (Point.Y - Polygon[i].Y) / (Polygon[j].Y - Polygon[i].Y) + Polygon[i].X)
		{
			bInside = !bInside;
		}
	}

	return bInside;
}

bool IsActorInArea(AActor* Actor, TArray<FVector2D>& Corners, const float MinZ, const float MaxZ) {
	return MinZ <= Actor->GetActorLocation().Z && Actor->GetActorLocation().Z <= MaxZ && IsPointInPolygon(FVector2D(Actor->GetActorLocation().X, Actor->GetActorLocation().Y), Corners);
}

void AAAEquipment::GetAllActorsInArea(TArray<AActor*>& OutActors) {
	for (TActorIterator<AFGBuildable> ActorIt(GetWorld()); ActorIt; ++ActorIt) {
		if (IsActorInArea(*ActorIt, this->AreaCorners, this->AreaMinZ, this->AreaMaxZ)) {
			OutActors.Add(*ActorIt);
		}
	}
}

AFGPlayerController* AAAEquipment::GetOwningController() const
{
	return static_cast<AFGPlayerController*>(this->GetInstigatorCharacter()->GetController());
}
