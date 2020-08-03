#include "AADebug.h"

#include "AACornerIndicator.h"
#include "util/Logging.h"

AAACornerIndicator* AAADebug::AddDebugIndicator(UWorld* World, const FVector Location)
{
   const TSubclassOf<AAACornerIndicator> DebugIndicator = DebugIndicatorClass.TryLoadClass<AAACornerIndicator>();
   if(!DebugIndicator)
   {
      SML::Logging::error(TEXT("Could not load AA debug indicator class"));
      return nullptr;
   }
   return World->SpawnActor<AAACornerIndicator>(DebugIndicator, Location, FRotator::ZeroRotator);
}

FSoftClassPath AAADebug::DebugIndicatorClass = FSoftClassPath("/Game/AreaActions/Indicators/Corner/CornerIndicator.CornerIndicator_C");