#pragma once

#include "Engine/LocalPlayer.h"
#include "GameFramework/Info.h"
#include "AASelectionActor.generated.h"

UENUM(BlueprintType)
enum class EAASelectionMode : uint8 {
	SM_Corner UMETA(DisplayName="Corner"),
	SM_Bottom UMETA(DisplayName="Bottom"),
	SM_Top UMETA(DisplayName="Top"),
	SM_Building UMETA(DisplayName="Building"),
	SM_MAX UMETA(Hidden)
};

UCLASS(Blueprintable)
class AREAACTIONS_API AAASelectionActor : public AInfo
{
	GENERATED_BODY()
public:
	AAASelectionActor();
	virtual void EnableInput(APlayerController* PlayerController) override;

	void PrimaryFire();
	void SecondaryFire();

	UFUNCTION(BlueprintCallable)
	void SetSelectionMode(const EAASelectionMode Mode) { this->SelectionMode = Mode; }
	
	UFUNCTION(BlueprintCallable)
	EAASelectionMode GetSelectionMode() const { return this->SelectionMode; }

	UFUNCTION(BlueprintCallable)
	void SetAreaActionsComponent(class UAAAreaActionsComponent* Subsystem) { this->AreaActionsComponent = Subsystem; }
	
	UFUNCTION(BlueprintCallable)
	class UAAAreaActionsComponent* GetAreaActionsComponent() const { return this->AreaActionsComponent; }

	EAASelectionMode SelectionMode;

	UPROPERTY()
	class UAAAreaActionsComponent* AreaActionsComponent;
};