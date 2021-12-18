#pragma once

#include "Engine/LocalPlayer.h"
#include "GameFramework/Info.h"
#include "AASelectionActor.generated.h"

UENUM(BlueprintType)
enum EAASelectionMode {
	SM_Corner UMETA(DisplayName="Corner"),
	SM_Bottom UMETA(DisplayName="Bottom"),
	SM_Top UMETA(DisplayName="Top"),
	SM_Building UMETA(DisplayName="Building"),
	SM_MAX UMETA(Hidden)
};

UCLASS()
class AREAACTIONS_API AAASelectionActor : public AInfo
{
	GENERATED_BODY()
public:
	AAASelectionActor();
	virtual void EnableInput(APlayerController* PlayerController) override;

	void PrimaryFire();

	UFUNCTION(BlueprintCallable)
	void SetSelectionMode(const EAASelectionMode Mode) { this->SelectionMode = Mode; }
	
	UFUNCTION(BlueprintCallable)
	EAASelectionMode GetSelectionMode() const { return this->SelectionMode; }

	EAASelectionMode SelectionMode;
};