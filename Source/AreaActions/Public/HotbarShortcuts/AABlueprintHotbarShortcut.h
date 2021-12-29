// Copyright Coffee Stain Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FGHotbarShortcut.h"
#include "AABlueprint.h"
#include "AABlueprintHotbarShortcut.generated.h"

/**
 * 
 */
UCLASS()
class AREAACTIONS_API UAABlueprintHotbarShortcut : public UFGHotbarShortcut
{
	GENERATED_BODY()

public:
	/** Decide on what properties to replicate */
	virtual void GetLifetimeReplicatedProps( TArray<FLifetimeProperty>& OutLifetimeProps ) const override;

	/** Get the header of the blueprint we want to activate when activating this shortcut */
	UFUNCTION( BlueprintPure, Category="AreaActions|Shortcut" )
	FAABlueprintHeader GetBlueprintHeader() const{ return mBlueprintHeaderToActivate; }

	/** Get the filename of the blueprint we want to activate when activating this shortcut */
	UFUNCTION( BlueprintPure, Category="AreaActions|Shortcut" )
	FString GetBlueprintFileName() const{ return mBlueprintFileNameToActivate; }

	/** Set the header of the blueprint of the current shortcut
	 * @param BlueprintHeader - null is valid, then we won't have any shortcut show up
	 **/
	UFUNCTION( BlueprintCallable, Category="AreaActions|Shortcut" )
	void SetBlueprintHeader(FAABlueprintHeader BlueprintHeader );

	/** Set the filename of the blueprint of the current shortcut
	 * @param BlueprintFileName - null is valid, then we won't have any shortcut show up
	 **/
	UFUNCTION( BlueprintCallable, Category="AreaActions|Shortcut" )
	void SetBlueprintFileName( FString BlueprintFileName );

	//~ Begin UFGHotbarShortcut interface
	virtual bool IsValidShortcut_Implementation( class AFGPlayerController* owner ) const override;
	virtual UTexture2D* GetDisplayImage_Implementation() const override;
	virtual bool IsActive_Implementation( class AFGPlayerController* owner ) const override;
	//~ End UFGHotbarShortcut interface
	protected:
	UFUNCTION()
	void OnRep_BlueprintHeader();
	UFUNCTION()
	void OnRep_BlueprintFileName();
protected:
	UPROPERTY( ReplicatedUsing=OnRep_BlueprintHeader, SaveGame )
	FAABlueprintHeader mBlueprintHeaderToActivate;
	
	UPROPERTY( ReplicatedUsing=OnRep_BlueprintFileName, SaveGame )
	FString mBlueprintFileNameToActivate;
};
