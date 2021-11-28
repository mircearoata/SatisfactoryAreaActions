#pragma once

#include "AACornerIndicator.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AADebug.generated.h"

UCLASS(Abstract, hidecategories = (Rendering, Replication, Input, Actor, Collision, "Actor Tick", LOD, Cooking))
class AREAACTIONS_API AAADebug : public AActor
{
    GENERATED_BODY()
public:    
    static AAACornerIndicator* AddDebugIndicator(UWorld* World, FVector Location);

private:
    static FSoftClassPath DebugIndicatorClass;
};
