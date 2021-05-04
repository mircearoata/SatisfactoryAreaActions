// Fill out your copyright notice in the Description page of Project Settings.

#include "AAMigrationSubsystem.h"
#include "AASubsystemManager.h"
#include "EngineUtils.h"
#include "Buildables/FGBuildable.h"
#include "Equipment//FGBuildGun.h"
#include "Resources/FGBuildingDescriptor.h"
#include "FGRecipeManager.h"

bool AAAMigrationSubsystem::ShouldSave_Implementation() const {
	return true;
}

USTRUCT()
struct FBuiltWith
{
	TMap<TSubclassOf<UFGRecipe>, int32> RecipeCount;
};

void AAAMigrationSubsystem::BeginPlay()
{
	if(!bFixedOldCopyBuildings)
	{
		// Use the most used recipe in the save for each building
		TMap<TSubclassOf<AFGBuildable>, FBuiltWith> BuiltWithPossibilities;
		for(TActorIterator<AFGBuildable> BuildableIt(GetWorld()); BuildableIt; ++BuildableIt)
		{
			TSubclassOf<UFGRecipe> Recipe = BuildableIt->GetBuiltWithRecipe();
			if(*Recipe)
				BuiltWithPossibilities.FindOrAdd(BuildableIt->GetClass()).RecipeCount.FindOrAdd(Recipe)++;
		}
		TMap<TSubclassOf<AFGBuildable>, TSubclassOf<UFGRecipe>> BuiltWith;
		for(auto BuildingRecipes : BuiltWithPossibilities)
		{
			TSubclassOf<UFGRecipe> BestRecipe;
			int32 BestCount = 0;
			for(const auto RecipeCount : BuildingRecipes.Value.RecipeCount)
			{
				if(RecipeCount.Value > BestCount)
				{
					BestRecipe = RecipeCount.Key;
					BestCount = RecipeCount.Value;
				}
			}
			BuiltWith.Add(BuildingRecipes.Key, BestRecipe);
		}

		// get recipes to fallback to the first recipe that produces it
		TArray<TSubclassOf<UFGRecipe>> AllRecipes;
		AFGRecipeManager::Get(GetWorld())->GetAvailableRecipesForProducer(AFGBuildGun::StaticClass(), AllRecipes);
		
		for(TActorIterator<AFGBuildable> BuildableIt(GetWorld()); BuildableIt; ++BuildableIt)
		{
			if(!*BuildableIt->GetBuiltWithRecipe())
			{
				if(!BuiltWith.Find(BuildableIt->GetClass()))
				{
					TSubclassOf<UFGRecipe> FoundRecipe;
					// check list of recipes and cache to BuiltWith
					for(TSubclassOf<UFGRecipe> Recipe : AllRecipes)
					{
						TArray<FItemAmount> Products = UFGRecipe::GetProducts(Recipe);
						for(FItemAmount Product : Products)
						{
							if((*Product.ItemClass)->IsChildOf(UFGBuildingDescriptor::StaticClass()))
							{
								TSubclassOf<UFGBuildingDescriptor> BuildingDescriptor(*Product.ItemClass);
								if(UFGBuildingDescriptor::GetBuildableClass(BuildingDescriptor) == BuildableIt->GetClass())
								{
									FoundRecipe = Recipe;
									break;
								}
							}
						}
						if(FoundRecipe) break;
					}
					BuiltWith.Add(BuildableIt->GetClass(), FoundRecipe);
				}
				BuildableIt->SetBuiltWithRecipe(BuiltWith[BuildableIt->GetClass()]);
			}
		}
			
		bFixedOldCopyBuildings = true;
	}
}

AAAMigrationSubsystem* AAAMigrationSubsystem::Get(UObject* WorldContextObject) {
	return AAASubsystemManager::Get(WorldContextObject)->GetMigrationSubsystem();
}
