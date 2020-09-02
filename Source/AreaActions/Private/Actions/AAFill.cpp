﻿#include "AAFill.h"

#include "AAEquipment.h"
#include "FGOutlineComponent.h"

AAAFill::AAAFill() {
    this->CopyBuildingsComponent = CreateDefaultSubobject<UAACopyBuildingsComponent>(TEXT("CopyBuildings"));
}

void AAAFill::Run_Implementation() {
    TArray<AFGBuildable*> BuildingsWithIssues;
    if (!this->CopyBuildingsComponent->SetActors(this->Actors, BuildingsWithIssues)) {
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
        FVector Offset = FVector(x * AreaSize.X + Border.X, y * AreaSize.Y + Border.Y, z * AreaSize.Z + Border.Z + Ramp.X * x + Ramp.Y * y);
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
    this->CopyBuildingsComponent->Finish();
    this->Done();
}

FFillAxis FFillAxis::None = FFillAxis();