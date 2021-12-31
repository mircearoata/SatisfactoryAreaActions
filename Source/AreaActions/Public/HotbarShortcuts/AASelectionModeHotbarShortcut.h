// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AASelectionActor.h"
#include "FGHotbarShortcut.h"
#include "AASelectionModeHotbarShortcut.generated.h"

/**
 * 
 */
UCLASS()
class AREAACTIONS_API UAASelectionModeHotbarShortcut : public UFGHotbarShortcut
{
	GENERATED_BODY()

public:
	/** Decide on what properties to replicate */
	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override;

	/** Get the action we want to activate when activating this shortcut */
	UFUNCTION( BlueprintPure, Category="Shortcut" )
	EAASelectionMode GetSelectionMode() const{ return mSelectionModeToActivate; }

	/** Set the action of the current shortcut, the action will be saved
	 * @param SelectionMode - null is valid, then we won't have any shortcut show up
	 **/
	UFUNCTION( BlueprintCallable, Category="Shortcut" )
	void SetSelectionMode(EAASelectionMode SelectionMode);

	//~ Begin UFGHotbarShortcut interface
	virtual void Execute_Implementation( class AFGPlayerController* owner ) override;
	virtual bool IsValidShortcut_Implementation( class AFGPlayerController* owner ) const override;
	virtual bool IsActive_Implementation( class AFGPlayerController* owner ) const override;
	//~ End UFGHotbarShortcut interface
protected:
	UFUNCTION()
	void OnRep_SelectionMode();
protected:
	UPROPERTY( ReplicatedUsing=OnRep_SelectionMode, SaveGame )
	EAASelectionMode mSelectionModeToActivate;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AAASelectionActor> SelectionActorClass;
};
