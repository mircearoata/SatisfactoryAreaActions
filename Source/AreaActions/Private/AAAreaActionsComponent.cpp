#include "AAAreaActionsComponent.h"

#include "AASelectionActor.h"
#include "Equipment/FGBuildGun.h"
#include "FGLocalPlayer.h"
#include "Buildables/FGBuildable.h"
#include "FGOutlineComponent.h"
#include "FGPlayerController.h"
#include "Actions/AALoadBlueprint.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "UI/FGGameUI.h"

UAAAreaActionsComponent::UAAAreaActionsComponent() : Super() {
	this->AreaMinZ = MinZ;
	this->AreaMaxZ = MaxZ;
	this->CornerIndicatorClass = FSoftClassPath(TEXT("/AreaActions/Indicators/Corner/CornerIndicator.CornerIndicator_C")).TryLoadClass<AAACornerIndicator>();
	this->WallIndicatorClass = FSoftClassPath(TEXT("/AreaActions/Indicators/Wall/WallIndicator.WallIndicator_C")).TryLoadClass<AAAWallIndicator>();
	this->HeightIndicatorClass = FSoftClassPath(TEXT("/AreaActions/Indicators/Height/HeightIndicator.HeightIndicator_C")).TryLoadClass<AAAHeightIndicator>();
}

void UAAAreaActionsComponent::BeginPlay()
{
	Super::BeginPlay();
	GetOuterAFGBuildGun()->mOnStateChanged.AddDynamic(this, &UAAAreaActionsComponent::OnBuildGunStateChanged);
}

void UAAAreaActionsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetOuterAFGBuildGun()->mOnStateChanged.RemoveDynamic(this, &UAAAreaActionsComponent::OnBuildGunStateChanged);
	for(AAACornerIndicator*& Indicator : CornerIndicators)
	{
		Indicator->Destroy();
	}
	for(AAAWallIndicator*& Indicator : WallIndicators)
	{
		Indicator->Destroy();
	}
	if(TopIndicator)
	{
		TopIndicator->Destroy();
		TopIndicator = nullptr;
	}
	if(BottomIndicator)
	{
		BottomIndicator->Destroy();
		BottomIndicator = nullptr;
	}
	if (CurrentAction)
	{
		CurrentAction->Cancel();
	}
	if(SelectionActor)
	{
		SelectionActor->Destroy();
		SelectionActor = nullptr;
	}
}

void UAAAreaActionsComponent::SelectMap() {
	this->ClearSelection();
	AddCorner(FVector2D(-350000.0, -350000.0));
	AddCorner(FVector2D(450000.0, -350000.0));
	AddCorner(FVector2D(450000.0, 450000.0));
	AddCorner(FVector2D(-350000.0, 450000.0));
	UpdateHeight();
}

void UAAAreaActionsComponent::ClearSelection() {
	for (int i = AreaCorners.Num() - 1; i >= 0; i--) {
		RemoveCorner(i);
	}
	AreaMinZ = MinZ;
	AreaMaxZ = MaxZ;
	UpdateHeight();
	ExtraActors.Empty();
	UpdateExtraActors();
}

bool UAAAreaActionsComponent::RaycastMouseWithRange(FHitResult& OutHitResult, const bool bIgnoreCornerIndicators, const bool bIgnoreWallIndicators, const bool bIgnoreHeightIndicators, const TArray<AActor*> OtherIgnoredActors) const
{
	TArray<AActor*> IgnoredActors;

	if (bIgnoreCornerIndicators) {
		IgnoredActors.Append(CornerIndicators);
	}
	if (bIgnoreWallIndicators) {
		IgnoredActors.Append(WallIndicators);
	}
	if (bIgnoreHeightIndicators) {
		IgnoredActors.Add(TopIndicator);
		IgnoredActors.Add(BottomIndicator);
	}

	APlayerController* PlayerController = GetPlayerController();
	APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;

	const FVector CameraLocation = CameraManager->GetCameraLocation();
	const FVector CameraDirection = CameraManager->GetActorForwardVector();

	const FVector LineTraceStart = CameraLocation + CameraDirection * (GetPlayerCharacter()->GetCapsuleComponent()->GetScaledCapsuleRadius() + 5);
	const FVector LineTraceEnd = CameraLocation + CameraDirection * MaxRaycastDistance;

	FCollisionQueryParams Params = FCollisionQueryParams::DefaultQueryParam;
	Params.AddIgnoredActors(IgnoredActors);
	Params.AddIgnoredActors(OtherIgnoredActors);
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel2); // Hologram
	ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel4); // Clearance
	ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel8); // HologramClearance
	return GetWorld()->LineTraceSingleByObjectType(OutHitResult, LineTraceStart, LineTraceEnd, ObjectParams, Params);
}

void UAAAreaActionsComponent::UpdateHeight() {
	for (AAACornerIndicator* Indicator : this->CornerIndicators) {
		Indicator->UpdateHeight(this->AreaMinZ, this->AreaMaxZ);
	}
	for (AAAWallIndicator* Indicator : this->WallIndicators) {
		Indicator->UpdateHeight(this->AreaMinZ, this->AreaMaxZ);
	}
	if(!this->BottomIndicator)
	{
		this->BottomIndicator = GetWorld()->SpawnActor<AAAHeightIndicator>(HeightIndicatorClass, FVector(0, 0, this->AreaMinZ), FRotator::ZeroRotator);
		this->BottomIndicator->SetIndicatorType(Bottom);
	}
	this->BottomIndicator->UpdateHeight(this->AreaMinZ, this->AreaMaxZ);
	this->BottomIndicator->SetActorHiddenInGame(this->AreaMinZ == MinZ);
	if(!this->TopIndicator)
	{
		this->TopIndicator = GetWorld()->SpawnActor<AAAHeightIndicator>(HeightIndicatorClass, FVector(0, 0, this->AreaMaxZ), FRotator::ZeroRotator);
		this->TopIndicator->SetIndicatorType(Top);
	}
	this->TopIndicator->UpdateHeight(this->AreaMinZ, this->AreaMaxZ);
	this->TopIndicator->SetActorHiddenInGame(this->AreaMaxZ == MaxZ);
}

AAAWallIndicator* UAAAreaActionsComponent::CreateWallIndicator(const FVector2D From, const FVector2D To) const
{
	const FVector2D Middle = (From + To) / 2;
	const float Length = FVector::Dist(FVector(From, 0), FVector(To, 0));
	const float Rotation = FVector((To - From), 0).Rotation().Yaw;

	AAAWallIndicator* Indicator = GetWorld()->SpawnActor<AAAWallIndicator>(WallIndicatorClass, FVector(Middle, 0), FRotator(0, Rotation, 0));
	Indicator->SetActorScale3D(FVector(Length, 1, 1));
	Indicator->UpdateHeight(this->AreaMinZ, this->AreaMaxZ);
	return Indicator;
}

AAACornerIndicator* UAAAreaActionsComponent::CreateCornerIndicator(const FVector2D Location) const
{
	AAACornerIndicator* Indicator = GetWorld()->SpawnActor<AAACornerIndicator>(CornerIndicatorClass, FVector(Location, 0), FRotator::ZeroRotator);
	Indicator->UpdateHeight(this->AreaMinZ, this->AreaMaxZ);
	return Indicator;
}

void UAAAreaActionsComponent::AddCorner(const FVector2D Location) {
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

void UAAAreaActionsComponent::RemoveCorner(const int CornerIdx) {
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

void UAAAreaActionsComponent::UpdateExtraActors() const
{
	UFGOutlineComponent::Get(GetWorld())->ShowDismantlePendingMaterial(this->ExtraActors);
}

void UAAAreaActionsComponent::DelayedUpdateExtraActors()
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UAAAreaActionsComponent::UpdateExtraActors);
}

void UAAAreaActionsComponent::OnBuildGunStateChanged(EBuildGunState NewState)
{
	if (NewState != EBuildGunState::BGS_MENU)
	{
		for(AAACornerIndicator*& Indicator : CornerIndicators)
			Indicator->Destroy();
		CornerIndicators.Empty();
		
		for(AAAWallIndicator*& Indicator : WallIndicators)
			Indicator->Destroy();
		WallIndicators.Empty();

		AreaCorners.Empty();

		AreaMinZ = MinZ;
		AreaMaxZ = MaxZ;
		UpdateHeight();

		ExtraActors.Empty();
		DelayedUpdateExtraActors();

		if (CurrentAction)
		{
			CurrentAction->Cancel();
		}
		if(SelectionActor)
		{
			SelectionActor->Destroy();
			SelectionActor = nullptr;
		}
	}
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

bool UAAAreaActionsComponent::RunAction(const TSubclassOf<AAAAction> ActionClass, FText& Error) {
	if (this->AreaCorners.Num() < 3 && this->ExtraActors.Num() == 0) {
		// TODO: Blueprint placing should not be an action
		if(!ActionClass->IsChildOf(AAALoadBlueprint::StaticClass()))
		{
			Error = AreaNotSetMessage;
			return false;
		}
	}
	if (this->CurrentAction) {
		Error = ConflictingActionRunningMessage;
		return false;
	}
	TArray<AActor*> ActorsInArea;
	this->GetAllActorsInArea(ActorsInArea);
	ActorsInArea.Append(this->ExtraActors);
	FVector Middle;
	GetMiddleOfActors(ActorsInArea, Middle);
	FRotator Rotation;
	GetMostCommonRotation(ActorsInArea, Rotation);

	this->CurrentAction = GetWorld()->SpawnActor<AAAAction>(ActionClass, Middle, Rotation);
	this->CurrentAction->SetAreaActionsComponent(this);
	this->CurrentAction->EnableInput(GetPlayerController());
	this->CurrentAction->SetActors(ActorsInArea);
	this->CurrentAction->Run();
	return true;
}

void UAAAreaActionsComponent::ActionDone() {
	this->CurrentAction->Destroy();
	this->CurrentAction = nullptr;
	this->ShowBuildMenu();
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

void UAAAreaActionsComponent::GetAllActorsInArea(TArray<AActor*>& OutActors) {
	for (TActorIterator<AFGBuildable> ActorIt(GetWorld()); ActorIt; ++ActorIt) {
		if (IsActorInArea(*ActorIt, this->AreaCorners, this->AreaMinZ, this->AreaMaxZ)) {
			OutActors.Add(*ActorIt);
		}
	}
}

void UAAAreaActionsComponent::ToggleBuildMenu()
{
	UFGBuildGunState* MenuState = GetPlayerCharacter()->GetBuildGun()->GetBuildGunStateFor(EBuildGunState::BGS_MENU);
	if(MenuState->IsActive())
		MenuState->EndState();
	else
		MenuState->BeginState();
}

void UAAAreaActionsComponent::HideBuildMenu()
{
	UFGBuildGunState* MenuState = GetPlayerCharacter()->GetBuildGun()->GetBuildGunStateFor(EBuildGunState::BGS_MENU);
	if(MenuState->IsActive())
		MenuState->EndState();
}

void UAAAreaActionsComponent::ShowBuildMenu()
{
	UFGBuildGunState* MenuState = GetPlayerCharacter()->GetBuildGun()->GetBuildGunStateFor(EBuildGunState::BGS_MENU);
	if(!MenuState->IsActive())
		MenuState->BeginState();
}
