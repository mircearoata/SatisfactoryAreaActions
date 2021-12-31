#include "Actions/AASaveBlueprint.h"

#include "AAAreaActionsComponent.h"
#include "AABlueprint.h"
#include "AABlueprintSystem.h"
#include "ImageUtils.h"

AAASaveBlueprint::AAASaveBlueprint() : Super()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent"));
	SceneCaptureComponent->bCaptureEveryFrame = false;
	SceneCaptureComponent->bCaptureOnMovement = false;
	SceneCaptureComponent->SetupAttachment(RootComponent);
	SceneCaptureComponent->SetRelativeTransform(FTransform(FRotator(-45, 0, 0), FVector(0, 0, 800), FVector::OneVector));
}

void AAASaveBlueprint::Run_Implementation()
{
	TArray<AActor*> ActorsWithIssues;
	Blueprint = UAABlueprint::FromRootSet(this, this->Actors, ActorsWithIssues);
	if (!Blueprint) {
		// Error handling
	}
	else {
		TArray<AActor*> HideActors;
		HideActors.Append(AreaActionsComponent->GetCornerIndicators());
		HideActors.Append(AreaActionsComponent->GetWallIndicators());
		HideActors.Add(AreaActionsComponent->GetTopIndicator());
		HideActors.Add(AreaActionsComponent->GetBottomIndicator());
		for(AActor* Actor : HideActors) {
			SceneCaptureComponent->HideActorComponents(Actor, true);
		}
		FAARotatedBoundingBox BoundingBox = Blueprint->GetBoundingBox();
		BoundingBox.Center = GetActorLocation();
		BoundingBox.Rotation = GetActorRotation();
		SceneCaptureComponent->SetWorldRotation(BoundingBox.Rotation + FRotator(-FMath::Asin(1/FMath::Sqrt(3)), 45, 0));
		FVector RelativePosition = BoundingBox.Rotation.RotateVector(BoundingBox.Extents);
		RelativePosition += FVector(400, 400, 400);
		RelativePosition.X = -RelativePosition.X;
		RelativePosition.Y = -RelativePosition.Y;
		SceneCaptureComponent->SetWorldLocation(BoundingBox.Center + RelativePosition);
		SceneCaptureComponent->CaptureScene();
		FBufferArchive Buffer;
		FImageUtils::ExportRenderTarget2DAsPNG(SceneCaptureComponent->TextureTarget, Buffer);
		Blueprint->SetIconBuffer(Buffer);
		this->ShowSelectBlueprintWidget();
	}
}

void AAASaveBlueprint::NameSelected(const FString BlueprintName)
{
	Blueprint->SetName(BlueprintName);
	GetGameInstance()->GetSubsystem<UAABlueprintSystem>()->SaveBlueprint(BlueprintName, Blueprint);
	this->Done();
}
