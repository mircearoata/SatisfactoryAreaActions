// Fill out your copyright notice in the Description page of Project Settings.

#include "AAEquipment.h"

#include "UI/FGGameUI.h"
#include "AALocalPlayerSubsystem.h"
#include "FGCharacterPlayer.h"
#include "FGPlayerController.h"
#include "Buildables/FGBuildable.h"

void AAAEquipment::Equip(class AFGCharacterPlayer* Character) {
	Super::Equip(Character);
	
	EnableInput(GetOwningController());
	if(!InputComponent->HasBindings())
	{
		InputComponent->BindAction("PrimaryFire", EInputEvent::IE_Pressed, this, &AAAEquipment::PrimaryFire);
		InputComponent->BindAction("SecondaryFire", EInputEvent::IE_Pressed, this, &AAAEquipment::SecondaryFire);
	}
	LocalPlayerSubsystem = GetOwningController()->GetLocalPlayer()->GetSubsystem<UAALocalPlayerSubsystem>();
	LocalPlayerSubsystem->OnActionDone.AddDynamic(this, &AAAEquipment::OnActionDone);
	UpdateDisplayedActionWidget();
	if(LocalPlayerSubsystem->CurrentAction)
	{
		LocalPlayerSubsystem->CurrentAction->EquipmentEquipped(this);
	}
}

void AAAEquipment::UnEquip() {
	DisableInput(GetOwningController());
	LocalPlayerSubsystem->OnActionDone.RemoveDynamic(this, &AAAEquipment::OnActionDone);
	if(LocalPlayerSubsystem->CurrentAction)
	{
		LocalPlayerSubsystem->CurrentAction->EquipmentUnEquipped();
	}
	Super::UnEquip();
}

void AAAEquipment::AddWidget(UFGInteractWidget* Widget)
{
	Widget->SetOwningPlayer(GetOwningController());
	AFGHUD* Hud = static_cast<AFGHUD*>(GetOwningController()->GetHUD());
	Hud->GetGameUI()->PopAllWidgets();
	Hud->GetGameUI()->AddInteractWidget(Widget);
	Hud->GetGameUI()->PushWidget(Widget);
}

void AAAEquipment::AddActionWidget(UWidget* Widget)
{
	LocalPlayerSubsystem->ActionWidgets.Add(Widget);
	UpdateDisplayedActionWidget();
}

void AAAEquipment::RemoveActionWidget(UWidget* Widget)
{
	LocalPlayerSubsystem->ActionWidgets.Add(Widget);
	UpdateDisplayedActionWidget();
}

void AAAEquipment::PrimaryFire() {
	FHitResult HitResult;
	switch (this->SelectionMode) {
	case EAASelectionMode::SM_Corner:
		if (RaycastMouseWithRange(HitResult, false, true, true)) {
			if (HitResult.Actor->IsA<AAACornerIndicator>()) {
				AAACornerIndicator* HitCorner = static_cast<AAACornerIndicator*>(HitResult.Actor.Get());
				const int CornerIdx = LocalPlayerSubsystem->CornerIndicators.Find(HitCorner);
				LocalPlayerSubsystem->RemoveCorner(CornerIdx);
			}
			else {
				LocalPlayerSubsystem->AddCorner(FVector2D(HitResult.Location.X, HitResult.Location.Y));
			}
		}
		break;
	case EAASelectionMode::SM_Bottom:
		if (RaycastMouseWithRange(HitResult, false, true, false)) {
			if (HitResult.Actor == LocalPlayerSubsystem->BottomIndicator) {
				LocalPlayerSubsystem->AreaMinZ = LocalPlayerSubsystem->MinZ;
			}
			else {
				LocalPlayerSubsystem->AreaMinZ = HitResult.Location.Z;
				if (LocalPlayerSubsystem->AreaMaxZ < LocalPlayerSubsystem->AreaMinZ) {
					const float Tmp = LocalPlayerSubsystem->AreaMinZ;
					LocalPlayerSubsystem->AreaMinZ = LocalPlayerSubsystem->AreaMaxZ;
					LocalPlayerSubsystem->AreaMaxZ = Tmp;
				}
			}
			LocalPlayerSubsystem->UpdateHeight();
		}
		break;
	case EAASelectionMode::SM_Top:
		if (RaycastMouseWithRange(HitResult, false, true, false)) {
			if (HitResult.Actor == LocalPlayerSubsystem->TopIndicator) {
				LocalPlayerSubsystem->AreaMaxZ = LocalPlayerSubsystem->MaxZ;
			}
			else {
				LocalPlayerSubsystem->AreaMaxZ = HitResult.Location.Z;
				if (LocalPlayerSubsystem->AreaMaxZ < LocalPlayerSubsystem->AreaMinZ) {
					const float Tmp = LocalPlayerSubsystem->AreaMinZ;
					LocalPlayerSubsystem->AreaMinZ = LocalPlayerSubsystem->AreaMaxZ;
					LocalPlayerSubsystem->AreaMaxZ = Tmp;
				}
			}
			LocalPlayerSubsystem->UpdateHeight();
		}
		break;
	case EAASelectionMode::SM_Building:
		if (RaycastMouseWithRange(HitResult, true, true, true)) {
			if (HitResult.Actor->IsA<AFGBuildable>()) {
				if (LocalPlayerSubsystem->ExtraActors.Contains(HitResult.Actor.Get())) {
					LocalPlayerSubsystem->ExtraActors.Remove(HitResult.Actor.Get());
				}
				else {
					LocalPlayerSubsystem->ExtraActors.Add(HitResult.Actor.Get());
				}
				LocalPlayerSubsystem->UpdateExtraActors();
			}
		}
		break;
	default:
		break;
	}
}

void AAAEquipment::SecondaryFire() {
	OpenMainWidget();
}

void AAAEquipment::SelectMap() {
	this->ClearSelection();
	LocalPlayerSubsystem->AddCorner(FVector2D(-350000.0, -350000.0));
	LocalPlayerSubsystem->AddCorner(FVector2D(450000.0, -350000.0));
	LocalPlayerSubsystem->AddCorner(FVector2D(450000.0, 450000.0));
	LocalPlayerSubsystem->AddCorner(FVector2D(-350000.0, 450000.0));
	LocalPlayerSubsystem->UpdateHeight();
}

void AAAEquipment::ClearSelection() {
	for (int i = LocalPlayerSubsystem->AreaCorners.Num() - 1; i >= 0; i--) {
		LocalPlayerSubsystem->RemoveCorner(i);
	}
	LocalPlayerSubsystem->AreaMinZ = LocalPlayerSubsystem->MinZ;
	LocalPlayerSubsystem->AreaMaxZ = LocalPlayerSubsystem->MaxZ;
	LocalPlayerSubsystem->UpdateHeight();
	LocalPlayerSubsystem->ExtraActors.Empty();
	LocalPlayerSubsystem->UpdateExtraActors();
}

bool AAAEquipment::RaycastMouseWithRange(FHitResult& OutHitResult, const bool bIgnoreCornerIndicators, const bool bIgnoreWallIndicators, const bool bIgnoreHeightIndicators, const TArray<AActor*> OtherIgnoredActors) const
{
	TArray<AActor*> IgnoredActors;

	if (bIgnoreCornerIndicators) {
		IgnoredActors.Append(LocalPlayerSubsystem->CornerIndicators);
	}
	if (bIgnoreWallIndicators) {
		IgnoredActors.Append(LocalPlayerSubsystem->WallIndicators);
	}
	if (bIgnoreHeightIndicators) {
		IgnoredActors.Add(LocalPlayerSubsystem->TopIndicator);
		IgnoredActors.Add(LocalPlayerSubsystem->BottomIndicator);
	}

	APlayerCameraManager* CameraManager = static_cast<AFGPlayerController*>(GetInstigatorCharacter()->GetController())->PlayerCameraManager;

	const FVector CameraLocation = CameraManager->GetCameraLocation();
	const FVector CameraDirection = CameraManager->GetActorForwardVector();

	const FVector LineTraceStart = CameraLocation + CameraDirection * (GetInstigatorCharacter()->GetCapsuleComponent()->GetScaledCapsuleRadius() + 5);
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

void AAAEquipment::RunAction(TSubclassOf<AAAAction> ActionClass)
{
	FText Error;
	if(!LocalPlayerSubsystem->RunAction(ActionClass, this, Error))
	{
		this->ShowMessageOk(WidgetTitle, Error);
	}
}

void AAAEquipment::OnActionDone()
{
	LocalPlayerSubsystem->ActionWidgets.Empty();
	UpdateDisplayedActionWidget();
}

AFGPlayerController* AAAEquipment::GetOwningController() const
{
	return static_cast<AFGPlayerController*>(this->GetInstigatorCharacter()->GetController());
}
