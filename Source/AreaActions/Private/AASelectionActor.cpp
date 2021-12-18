#include "AASelectionActor.h"

#include "Buildables/FGBuildable.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "AALocalPlayerSubsystem.h"

AAASelectionActor::AAASelectionActor() : Super()
{
}

void AAASelectionActor::EnableInput(APlayerController* PlayerController)
{
	Super::EnableInput(PlayerController);
	
	InputComponent->BindAction(TEXT("PrimaryFire"), IE_Pressed, this, &AAASelectionActor::PrimaryFire);
}

void AAASelectionActor::PrimaryFire() {
	UAALocalPlayerSubsystem* AALocalPlayerSubsystem = GetWorld()->GetFirstLocalPlayerFromController()->GetSubsystem<UAALocalPlayerSubsystem>();
	FHitResult HitResult;
	switch (this->SelectionMode) {
	case EAASelectionMode::SM_Corner:
		if (AALocalPlayerSubsystem->RaycastMouseWithRange(HitResult, false, true, true)) {
			if (HitResult.Actor->IsA<AAACornerIndicator>()) {
				AAACornerIndicator* HitCorner = static_cast<AAACornerIndicator*>(HitResult.Actor.Get());
				const int CornerIdx = AALocalPlayerSubsystem->CornerIndicators.Find(HitCorner);
				AALocalPlayerSubsystem->RemoveCorner(CornerIdx);
			}
			else {
				AALocalPlayerSubsystem->AddCorner(FVector2D(HitResult.Location.X, HitResult.Location.Y));
			}
		}
		break;
	case EAASelectionMode::SM_Bottom:
		if (AALocalPlayerSubsystem->RaycastMouseWithRange(HitResult, false, true, false)) {
			if (HitResult.Actor == AALocalPlayerSubsystem->BottomIndicator) {
				AALocalPlayerSubsystem->AreaMinZ = AALocalPlayerSubsystem->MinZ;
			}
			else {
				AALocalPlayerSubsystem->AreaMinZ = HitResult.Location.Z;
				if (AALocalPlayerSubsystem->AreaMaxZ < AALocalPlayerSubsystem->AreaMinZ) {
					const float Tmp = AALocalPlayerSubsystem->AreaMinZ;
					AALocalPlayerSubsystem->AreaMinZ = AALocalPlayerSubsystem->AreaMaxZ;
					AALocalPlayerSubsystem->AreaMaxZ = Tmp;
				}
			}
			AALocalPlayerSubsystem->UpdateHeight();
		}
		break;
	case EAASelectionMode::SM_Top:
		if (AALocalPlayerSubsystem->RaycastMouseWithRange(HitResult, false, true, false)) {
			if (HitResult.Actor == AALocalPlayerSubsystem->TopIndicator) {
				AALocalPlayerSubsystem->AreaMaxZ = AALocalPlayerSubsystem->MaxZ;
			}
			else {
				AALocalPlayerSubsystem->AreaMaxZ = HitResult.Location.Z;
				if (AALocalPlayerSubsystem->AreaMaxZ < AALocalPlayerSubsystem->AreaMinZ) {
					const float Tmp = AALocalPlayerSubsystem->AreaMinZ;
					AALocalPlayerSubsystem->AreaMinZ = AALocalPlayerSubsystem->AreaMaxZ;
					AALocalPlayerSubsystem->AreaMaxZ = Tmp;
				}
			}
			AALocalPlayerSubsystem->UpdateHeight();
		}
		break;
	case EAASelectionMode::SM_Building:
		if (AALocalPlayerSubsystem->RaycastMouseWithRange(HitResult, true, true, true)) {
			if (HitResult.Actor->IsA<AFGBuildable>()) {
				if (AALocalPlayerSubsystem->ExtraActors.Contains(HitResult.Actor.Get())) {
					AALocalPlayerSubsystem->ExtraActors.Remove(HitResult.Actor.Get());
				}
				else {
					AALocalPlayerSubsystem->ExtraActors.Add(HitResult.Actor.Get());
				}
				AALocalPlayerSubsystem->UpdateExtraActors();
			}
		}
		break;
	default:
		break;
	}
}
