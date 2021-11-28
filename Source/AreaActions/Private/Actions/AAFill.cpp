#include "Actions/AAFill.h"

#include "AAEquipment.h"
#include "FGOutlineComponent.h"
#include "FGPlayerController.h"
#include "Buildables/FGBuildableStorage.h"

AAAFill::AAAFill() {
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
		if(AAEquipment->RaycastMouseWithRange(HitResult, true, true, true, HologramsActors))
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
}

void AAAFill::PrimaryFire()
{
	if(bIsPlacing)
	{
		bIsPlacing = false;
		ScrollUpInputActionBinding->bConsumeInput = false;
		ScrollDownInputActionBinding->bConsumeInput = false;
	}
	AAEquipment->OpenMainWidget();
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

void AAAFill::EquipmentEquipped(AAAEquipment* Equipment)
{
    Super::EquipmentEquipped(Equipment);
    if(!InputComponent->HasBindings())
    {
        InputComponent->BindAction("PrimaryFire", EInputEvent::IE_Pressed, this, &AAAFill::PrimaryFire);
        InputComponent->BindAction("SecondaryFire", EInputEvent::IE_Pressed, this, &AAAFill::PrimaryFire);
        ScrollUpInputActionBinding = &InputComponent->BindAction("BuildGunScrollUp_PhotoModeFOVUp", EInputEvent::IE_Pressed, this, &AAAFill::ScrollUp);
        ScrollDownInputActionBinding = &InputComponent->BindAction("BuildGunScrollDown_PhotoModeFOVDown", EInputEvent::IE_Pressed, this, &AAAFill::ScrollDown);
        ScrollUpInputActionBinding->bConsumeInput = false;
        ScrollDownInputActionBinding->bConsumeInput = false;
    }
}

void AAAFill::Run_Implementation() {
    TArray<AFGBuildable*> BuildingsWithIssues;
    FText Error;
    if (!this->CopyBuildingsComponent->SetActors(this->Actors, BuildingsWithIssues, Error)) {
        if(!Error.IsEmpty())
        {
            FOnMessageOk MessageOk;
            MessageOk.BindDynamic(this, &AAAFill::Done);
            UWidget* MessageOkWidget = this->AAEquipment->CreateActionMessageOk(Error, MessageOk);
            this->AAEquipment->AddActionWidget(MessageOkWidget);
        }
        else
        {
            TArray<AActor*> AsActors;
            for (AFGBuildable* Building : BuildingsWithIssues) {
                AsActors.Add(Building);
            }
            UFGOutlineComponent::Get(GetWorld())->ShowDismantlePendingMaterial(AsActors);
            FOnMessageOk MessageOk;
            MessageOk.BindDynamic(this, &AAAFill::Done);
            const FText Message = FText::FromString(TEXT("Some buildings cannot be copied because they have connections to buildings outside the selected area. Remove the connections temporary, or include the connected buildings in the area. The buildings are highlighted."));
            UWidget* MessageOkWidget = this->AAEquipment->CreateActionMessageOk(Message, MessageOk);
            this->AAEquipment->AddActionWidget(MessageOkWidget);
        }
    }
    else {
        this->AreaSize = this->CopyBuildingsComponent->GetBounds().Extents * 2;
        this->ShowFillWidget();
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

void AAAFill::Finish()
{
    this->Preview();
    TArray<UFGInventoryComponent*> Inventories;
    TArray<FInventoryStack> MissingItems;
    AFGCharacterPlayer* Player = static_cast<AFGCharacterPlayer*>(this->AAEquipment->GetOwningController()->GetPawn());

    for(TActorIterator<AFGBuildableStorage> StorageIt(GetWorld()); StorageIt; ++StorageIt)
    {
        if(FVector::Distance(StorageIt->GetActorLocation(), Player->GetActorLocation()) < 10000)
        {
            Inventories.Add(StorageIt->GetInitialStorageInventory());
        }
    }
	
    Inventories.Add(Player->GetInventory());
    if(!this->CopyBuildingsComponent->Finish(Inventories, MissingItems))
    {
        FString MissingItemsString = TEXT("");
        for(const auto Stack : MissingItems)
        {
            MissingItemsString += FString::Printf(
                TEXT("%s%d %s"), MissingItemsString.Len() != 0 ? TEXT(", ") : TEXT(""),
                Stack.NumItems,
                *UFGItemDescriptor::GetItemName(Stack.Item.ItemClass).ToString());
        }
        FOnMessageOk MessageOk;
        MessageOk.BindDynamic(this, &AAAFill::RemoveMissingItemsWidget);
        const FText Message = FText::FromString(FString::Printf(TEXT("Missing items: %s"), *MissingItemsString));
        MissingItemsWidget = this->AAEquipment->CreateActionMessageOk(Message, MessageOk);
        this->AAEquipment->AddActionWidget(MissingItemsWidget);
    }
    else
        this->Done();
}

void AAAFill::RemoveMissingItemsWidget()
{
    this->AAEquipment->RemoveActionWidget(MissingItemsWidget);
    MissingItemsWidget = nullptr;
}

void AAAFill::EnterPlacing()
{
	ScrollUpInputActionBinding->bConsumeInput = true;
	ScrollDownInputActionBinding->bConsumeInput = true;
	bIsPlacing = true;
	AAEquipment->CloseMainWidget();
}

FFillAxis FFillAxis::None = FFillAxis();