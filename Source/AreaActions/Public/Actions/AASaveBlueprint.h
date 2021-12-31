#pragma once

#include "CoreMinimal.h"

#include "AAAction.h"
#include "AABlueprint.h"
#include "AASaveBlueprint.generated.h"

UCLASS(Abstract, Blueprintable)
class AREAACTIONS_API AAASaveBlueprint : public AAAAction
{
	GENERATED_BODY()

public:
	AAASaveBlueprint();
	
	UFUNCTION(BlueprintCallable)
	bool CreateBlueprintObject(TArray<AActor*>& ActorsWithIssues);

	UFUNCTION(BlueprintCallable)
	void CaptureIcon();
	
	UFUNCTION(BlueprintCallable)
	void SaveBlueprint(const FString BlueprintName);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USceneCaptureComponent2D* SceneCaptureComponent;

	UFUNCTION(BlueprintPure)
	FORCEINLINE UTexture2D* GetBlueprintIcon() { return Blueprint->GetIcon(); }
private:
	UPROPERTY()
	UAABlueprint* Blueprint;
};
