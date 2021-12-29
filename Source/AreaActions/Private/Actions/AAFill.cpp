#include "Actions/AAFill.h"

#include "AAAreaActionsComponent.h"
#include "AABlueprintFunctionLibrary.h"
#include "FGOutlineComponent.h"
#include "FGPlayerController.h"
#include "Buildables/FGBuildableStorage.h"

AAAFill::AAAFill() : Super() {
    this->CopyBuildingsComponent = CreateDefaultSubobject<UAACopyBuildingsComponent>(TEXT("CopyBuildings"));

	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AAAFill::Tick(float DeltaSeconds)
{
	if(bIsPlacing)
	{
		TArray<AFGBuildableHologram*> Holograms;
		CopyBuildingsComponent->GetAllPreviewHolograms(Holograms);
		TArray<AActor*> HologramsActors(MoveTemp(Holograms));
		FHitResult HitResult;
		if(AreaActionsComponent->RaycastMouseWithRange(HitResult, true, true, true, HologramsActors))
		{
			FVector Location = CopyBuildingsComponent->GetBounds().Rotation.UnrotateVector(HitResult.Location - CopyBuildingsComponent->GetBounds().Center);
			FVector CopyOffset = CopyBuildingsComponent->GetBounds().Extents * 2 + Border;
			FVector Corner = CopyBuildingsComponent->GetBounds().Extents;
			if(Location.X < 0)
				Corner.X = -Corner.X;
			if(Location.Y < 0)
				Corner.Y = -Corner.Y;
			if(Location.Z < 0)
				Corner.Z = -Corner.Z;
			FIntVector AxisCount = FIntVector((Location + Corner) / CopyOffset);
			if(FillDirection == EFillDirection::XY || FillDirection == EFillDirection::XZ || FillDirection == EFillDirection::XYZ)
				Count.X = FFillAxis(FGenericPlatformMath::Abs(AxisCount.X) + 1, AxisCount.X < 0);
			else
				Count.X = FFillAxis(1, false);
			if(FillDirection == EFillDirection::XY || FillDirection == EFillDirection::YZ || FillDirection == EFillDirection::XYZ)
				Count.Y = FFillAxis(FGenericPlatformMath::Abs(AxisCount.Y) + 1, AxisCount.Y < 0);
			else
				Count.Y = FFillAxis(1, false);
			if(FillDirection == EFillDirection::XZ || FillDirection == EFillDirection::YZ || FillDirection == EFillDirection::XYZ)
				Count.Z = FFillAxis(FGenericPlatformMath::Abs(AxisCount.Z) + 1, AxisCount.Z < 0);
			else
				Count.Z = FFillAxis(1, false);
		}
		Preview();
	}
	CopyBuildingsComponent->SetHologramState(CanFinish() ? EHologramMaterialState::HMS_OK : EHologramMaterialState::HMS_ERROR);
}

void AAAFill::EnableInput(APlayerController* PlayerController)
{
	Super::EnableInput(PlayerController);
	
	InputComponent->BindAction("PrimaryFire", EInputEvent::IE_Pressed, this, &AAAFill::PrimaryFire);
	InputComponent->BindAction("SecondaryFire", EInputEvent::IE_Pressed, this, &AAAFill::OpenMenu);
	ScrollUpInputActionBinding = &InputComponent->BindAction("BuildGunScrollUp_PhotoModeFOVUp", EInputEvent::IE_Pressed, this, &AAAFill::ScrollUp);
	ScrollDownInputActionBinding = &InputComponent->BindAction("BuildGunScrollDown_PhotoModeFOVDown", EInputEvent::IE_Pressed, this, &AAAFill::ScrollDown);
	ScrollUpInputActionBinding->bConsumeInput = false;
	ScrollDownInputActionBinding->bConsumeInput = false;
}

void AAAFill::PrimaryFire()
{
	if(bIsPlacing)
	{
		if(Finish())
		{
			bIsPlacing = false;
			ScrollUpInputActionBinding->bConsumeInput = false;
			ScrollDownInputActionBinding->bConsumeInput = false;
			Done();
		}
	}
}

void AAAFill::OpenMenu()
{
	bIsPlacing = false;
	ScrollUpInputActionBinding->bConsumeInput = false;
	ScrollDownInputActionBinding->bConsumeInput = false;
	AreaActionsComponent->ShowBuildMenu();
}

void AAAFill::ScrollUp()
{
	switch (FillDirection)
	{
	case EFillDirection::XY:
		FillDirection = EFillDirection::XZ;
		break;
	case EFillDirection::XZ:
		FillDirection = EFillDirection::YZ;
		break;
	case EFillDirection::YZ:
		FillDirection = EFillDirection::XYZ;
		break;
	case EFillDirection::XYZ:
		FillDirection = EFillDirection::XY; 
		break;
	}
}

void AAAFill::ScrollDown()
{
	switch (FillDirection)
	{
	case EFillDirection::XZ:
		FillDirection = EFillDirection::XY;
		break;
	case EFillDirection::YZ:
		FillDirection = EFillDirection::XZ;
		break;
	case EFillDirection::XYZ:
		FillDirection = EFillDirection::YZ;
		break;
	case EFillDirection::XY:
		FillDirection = EFillDirection::XYZ; 
		break;
	}
}

void AAAFill::OnCancel_Implementation()
{
    for(const auto Copy : CopyId)
        this->CopyBuildingsComponent->RemoveCopy(Copy.Value);
    CopyId.Empty();
}

void AAAFill::Preview()
{
    for(int32 x = 0; x < Count.X.Amount; x++)
    for(int32 y = 0; y < Count.Y.Amount; y++)
    for(int32 z = 0; z < Count.Z.Amount; z++)
    {
        if(x == 0 && y == 0 && z == 0)
            continue;
        FIntVector CopyNumber = FIntVector(x, y, z);
        FVector Offset = FVector(x * (AreaSize.X + Border.X), y * (AreaSize.Y + Border.Y), z * (AreaSize.Z + Border.Z) + Ramp.X * x + Ramp.Y * y);
        if(Count.X.Reversed)
            Offset.X = -Offset.X;
        if(Count.Y.Reversed)
            Offset.Y = -Offset.Y;
        if(Count.Z.Reversed)
            Offset.Z = -Offset.Z;
        if(!CopyId.Contains(CopyNumber))
            CopyId.Add(CopyNumber, this->CopyBuildingsComponent->AddCopy(Offset, FRotator::ZeroRotator));
        else
            this->CopyBuildingsComponent->MoveCopy(CopyId[CopyNumber], Offset, FRotator::ZeroRotator);
    }

    // Remove copies that are outside current settings
    TArray<FIntVector> ToRemove;
    for(auto Copy : CopyId)
        if(Copy.Key.X >= Count.X.Amount || Copy.Key.Y >= Count.Y.Amount || Copy.Key.Z >= Count.Z.Amount)
            ToRemove.Add(Copy.Key);
    for(FIntVector Remove : ToRemove)
    {
        this->CopyBuildingsComponent->RemoveCopy(CopyId[Remove]);
        CopyId.Remove(Remove);
    }
}


bool AAAFill::Finish()
{
	if(!CanFinish())
		return false;
	this->Preview();
	TMap<TSubclassOf<UFGItemDescriptor>, int32> MissingItemsMap;
	const bool ReturnValue = this->CopyBuildingsComponent->Finish(GetInventories(), MissingItemsMap);
	return ReturnValue;  
}

bool AAAFill::CanFinish() const
{
	TMap<TSubclassOf<UFGItemDescriptor>, int32> MissingItemsMap;
	return UAABlueprintFunctionLibrary::InventoriesHaveEnoughItems(GetInventories(), CopyBuildingsComponent->GetRequiredItems(), MissingItemsMap);
}

void AAAFill::RemoveMissingItemsWidget()
{
    MissingItemsWidget = nullptr;
}

void AAAFill::EnterPlacing()
{
	ScrollUpInputActionBinding->bConsumeInput = true;
	ScrollDownInputActionBinding->bConsumeInput = true;
	bIsPlacing = true;
	AreaActionsComponent->HideBuildMenu();
}

FFillAxis FFillAxis::None = FFillAxis();