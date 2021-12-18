#include "AreaActionsModule.h"

#include "FGPlayerController.h"
#include "Patching/NativeHookManager.h"

DEFINE_LOG_CATEGORY(LogAreaActions);
DEFINE_LOG_CATEGORY(LogGame);

void FAreaActionsModule::StartupModule() {
}


IMPLEMENT_GAME_MODULE(FAreaActionsModule, AreaActions);
