#include "AAFill.h"

#include "AAEquipment.h"
#include "FGOutlineComponent.h"

AAAFill::AAAFill() {
    this->CopyBuildingsComponent = CreateDefaultSubobject<UAACopyBuildingsComponent>(TEXT("CopyBuildings"));
}

void AAAFill::Run_Implementation() {
    TArray<AFGBuildable*> buildingsWithIssues;
    if (!this->CopyBuildingsComponent->SetActors(this->mActors, buildingsWithIssues)) {
        TArray<AActor*> asActors;
        for (AFGBuildable* building : buildingsWithIssues) {
            asActors.Add(building);
        }
        UFGOutlineComponent::Get(GetWorld())->ShowDismantlePendingMaterial(asActors);
        FOnMessageOk messageOk;
        messageOk.BindDynamic(this, &AAAFill::Done);
        FText message = FText::FromString(TEXT("Some buildings cannot be copied because they have connections to buildings outside the selected area. Remove the connections temporary, or include the connected buildings in the area. The buildings are highlighted."));
        this->mAAEquipment->ShowMessageOkDelegate(ActionName, message, messageOk);
    }
    else {
        this->AreaSize = this->CopyBuildingsComponent->GetBounds().Extents * 2;
        this->SetSettings(FFillCount(FFillAxis(10, true), FFillAxis(2, false), FFillAxis::None), FVector::ZeroVector, FVector::ZeroVector);
        this->Preview();
        FTimerDelegate TimerCallback;
        TimerCallback.BindLambda([=]()
        {
            this->SetSettings(FFillCount(FFillAxis(2, false), FFillAxis(10, true), FFillAxis(3, false)), FVector::ZeroVector, FVector::ZeroVector);
            this->Preview();
            this->Finish();
        });

        FTimerHandle Handle;
        GetWorld()->GetTimerManager().SetTimer(Handle, TimerCallback, 10.0f, false);
    }
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
        FVector Offset = FVector(x * AreaSize.X + Border.X, y * AreaSize.Y + Border.Y, z * AreaSize.Z + Border.Z);
        if(Count.X.Reversed)
            Offset.X = -Offset.X;
        if(Count.Y.Reversed)
            Offset.X = -Offset.Y;
        if(Count.Z.Reversed)
            Offset.X = -Offset.Z;
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