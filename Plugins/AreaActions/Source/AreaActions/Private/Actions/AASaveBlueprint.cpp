#include "Actions/AASaveBlueprint.h"

#include "AABlueprint.h"

void AAASaveBlueprint::Run_Implementation()
{
	TArray<AActor*> ActorsWithIssues;
	Blueprint = NewObject<UAABlueprint>(GetWorld());
	if (!Blueprint->SetRootSet(this->Actors, ActorsWithIssues)) {
		// Error handling
	}
	else {
		this->ShowSelectBlueprintWidget();
	}
}

void AAASaveBlueprint::NameSelected(const FString BlueprintName)
{
	Blueprint->SaveBlueprint(UAABlueprint::GetBlueprintPath(BlueprintName));
	this->Done();
}
