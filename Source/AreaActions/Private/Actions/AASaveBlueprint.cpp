#include "Actions/AASaveBlueprint.h"

#include "AABlueprint.h"
#include "AABlueprintSystem.h"

void AAASaveBlueprint::Run_Implementation()
{
	TArray<AActor*> ActorsWithIssues;
	Blueprint = UAABlueprint::FromRootSet(this, this->Actors, ActorsWithIssues);
	if (!Blueprint) {
		// Error handling
	}
	else {
		this->ShowSelectBlueprintWidget();
	}
}

void AAASaveBlueprint::NameSelected(const FString BlueprintName)
{
	Blueprint->SetName(BlueprintName);
	GetGameInstance()->GetSubsystem<UAABlueprintSystem>()->SaveBlueprint(UAABlueprintSystem::GetBlueprintPath(BlueprintName), Blueprint);
	this->Done();
}
