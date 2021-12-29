// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FGHotbarShortcut.h"
#include "AAActionHotbarShortcut.generated.h"

/**
 * 
 */
UCLASS()
class AREAACTIONS_API UAAActionHotbarShortcut : public UFGHotbarShortcut
{
	GENERATED_BODY()

public:
	/** Decide on what properties to replicate */
	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override;

	/** Get the action we want to activate when activating this shortcut */
	UFUNCTION( BlueprintPure, Category="Shortcut" )
	TSubclassOf< class AAAAction > GetAction() const{ return mActionToActivate; }

	/** Set the action of the current shortcut, the action will be saved
	 * @param action - null is valid, then we won't have any shortcut show up
	 **/
	UFUNCTION( BlueprintCallable, Category="Shortcut" )
	void SetAction( TSubclassOf< class AAAAction > Action );

	//~ Begin UFGHotbarShortcut interface
	virtual void Execute_Implementation( class AFGPlayerController* owner ) override;
	virtual bool IsValidShortcut_Implementation( class AFGPlayerController* owner ) const override;
	virtual UTexture2D* GetDisplayImage_Implementation() const override;
	virtual bool IsActive_Implementation( class AFGPlayerController* owner ) const override;
	//~ End UFGHotbarShortcut interface
protected:
	UFUNCTION()
	void OnRep_Action();
protected:
	UPROPERTY( ReplicatedUsing=OnRep_Action, SaveGame )
	TSubclassOf< class AAAAction > mActionToActivate;
};
