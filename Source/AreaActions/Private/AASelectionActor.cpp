#include "AASelectionActor.h"

#include "AABlueprintFunctionLibrary.h"
#include "AAAreaActionsComponent.h"
#include "Buildables/FGBuildable.h"
#include "AAAreaActionsComponent.h"

AAASelectionActor::AAASelectionActor() : Super()
{
}

void AAASelectionActor::EnableInput(APlayerController* PlayerController)
{
	Super::EnableInput(PlayerController);
	
	InputComponent->BindAction(TEXT("PrimaryFire"), IE_Pressed, this, &AAASelectionActor::PrimaryFire);	
	InputComponent->BindAction(TEXT("SecondaryFire"), IE_Pressed, this, &AAASelectionActor::SecondaryFire);
}

void AAASelectionActor::PrimaryFire() {
	FHitResult HitResult;
	switch (this->SelectionMode) {
	case EAASelectionMode::SM_Corner:
		if (AreaActionsComponent->RaycastMouseWithRange(HitResult, false, true, true)) {
			if (HitResult.Actor->IsA<AAACornerIndicator>()) {
				AAACornerIndicator* HitCorner = static_cast<AAACornerIndicator*>(HitResult.Actor.Get());
				const int CornerIdx = AreaActionsComponent->CornerIndicators.Find(HitCorner);
				AreaActionsComponent->RemoveCorner(CornerIdx);
			}
			else {
				AreaActionsComponent->AddCorner(FVector2D(HitResult.Location.X, HitResult.Location.Y));
			}
		}
		break;
	case EAASelectionMode::SM_Bottom:
		if (AreaActionsComponent->RaycastMouseWithRange(HitResult, false, true, false)) {
			if (HitResult.Actor == AreaActionsComponent->BottomIndicator) {
				AreaActionsComponent->AreaMinZ = AreaActionsComponent->MinZ;
			}
			else {
				AreaActionsComponent->AreaMinZ = HitResult.Location.Z;
				if (AreaActionsComponent->AreaMaxZ < AreaActionsComponent->AreaMinZ) {
					const float Tmp = AreaActionsComponent->AreaMinZ;
					AreaActionsComponent->AreaMinZ = AreaActionsComponent->AreaMaxZ;
					AreaActionsComponent->AreaMaxZ = Tmp;
				}
			}
			AreaActionsComponent->UpdateHeight();
		}
		break;
	case EAASelectionMode::SM_Top:
		if (AreaActionsComponent->RaycastMouseWithRange(HitResult, false, true, false)) {
			if (HitResult.Actor == AreaActionsComponent->TopIndicator) {
				AreaActionsComponent->AreaMaxZ = AreaActionsComponent->MaxZ;
			}
			else {
				AreaActionsComponent->AreaMaxZ = HitResult.Location.Z;
				if (AreaActionsComponent->AreaMaxZ < AreaActionsComponent->AreaMinZ) {
					const float Tmp = AreaActionsComponent->AreaMinZ;
					AreaActionsComponent->AreaMinZ = AreaActionsComponent->AreaMaxZ;
					AreaActionsComponent->AreaMaxZ = Tmp;
				}
			}
			AreaActionsComponent->UpdateHeight();
		}
		break;
	case EAASelectionMode::SM_Building:
		if (AreaActionsComponent->RaycastMouseWithRange(HitResult, true, true, true)) {
			if (HitResult.Actor->IsA<AFGBuildable>()) {
				if (AreaActionsComponent->ExtraActors.Contains(HitResult.Actor.Get())) {
					AreaActionsComponent->ExtraActors.Remove(HitResult.Actor.Get());
				}
				else {
					AreaActionsComponent->ExtraActors.Add(HitResult.Actor.Get());
				}
				AreaActionsComponent->UpdateExtraActors();
			}
		}
		break;
	default:
		break;
	}
}

void AAASelectionActor::SecondaryFire() {
	AreaActionsComponent->ToggleBuildMenu();
	this->Destroy();
}