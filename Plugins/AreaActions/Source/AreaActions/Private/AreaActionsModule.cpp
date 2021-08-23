#include "AreaActionsModule.h"

#include "FGPlayerController.h"
#include "UObject/CoreRedirects.h"

DEFINE_LOG_CATEGORY(LogAreaActions);

void FAreaActionsModule::StartupModule() {
#if !WITH_EDITOR && AA_DEBUG
    FWorldDelegates::OnWorldInitializedActors.AddLambda([=](const UWorld::FActorsInitializedParams Params){
        UWorld* World = Params.World;
        const bool bIsMenuWorld = (static_cast<AFGGameMode*>(World->GetAuthGameMode()))->IsMainMenuGameMode();
        if(bIsMenuWorld)
        {
            FTimerHandle Handle;
            World->GetTimerManager().SetTimer(Handle, [=]()
            {
                UFGSaveSystem* SaveSystem = UFGSaveSystem::Get(World);
                FOnEnumerateSaveGamesComplete EnumerateComplete;
                EnumerateComplete.BindLambda([=](bool success, const TArray<FSaveHeader>& constSaves, void* userdata)
                {
                    TArray<FSaveHeader> saves;
                    for(FSaveHeader save : constSaves)
                        saves.Add(save);
                    SaveSystem->SortSaves(saves, ESaveSortMode::SSM_Time, ESaveSortDirection::SSD_Descending);
                    static_cast<AFGPlayerController*>(World->GetFirstPlayerController())->GetAdminInterface()->LoadGame(false, saves[0]);
                });
                SaveSystem->EnumerateSaveGames(EnumerateComplete, nullptr);
            }, 5, false);
        }
    });
#endif
}


IMPLEMENT_GAME_MODULE(FAreaActionsModule, AreaActions);
