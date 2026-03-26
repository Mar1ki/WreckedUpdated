#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/LivingWorld/FortAthenaLivingWorldManager.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"
#include "Engine/Source/Runtime/FortniteGame/Public/LivingWorld/FortAthenaLivingWorldEvent.h"

static bool NeedsLoadForClient() {
    return true;
}


void FortAthenaLivingWorldManager::RequestEventGeneration(UObject* Object, FFrame* Frame)
{
    std::cout << "One of the event exec is called" << std::endl;
    RequestEventGenerationOG(Object, Frame);
}

void FortAthenaLivingWorldManager::TrySpawnEvent(UObject* Object, FFrame* Frame)
{
    std::cout << "TrySpawnEvent" << std::endl;
    TrySpawnEventOG(Object, Frame);
}


void FortAthenaLivingWorldManager::HandleGamePhaseStepChanged(UObject* Object, FFrame* Frame)
{
    if (!bEnableLWM) return;
    auto Logic = (UFortGameStateComponent_BattleRoyaleGamePhaseLogic*)Object;
    auto GamePhaseStep =  Logic->GamePhaseStep;

    if (GamePhaseStep == EAthenaGamePhaseStep::Warmup)
    {
        FortAthenaLivingWorldEvent::TrySpawnEvents();
    }
    std::cout << "GamePhaseStep Changed:" << (int*)GamePhaseStep << std::endl;
    for (auto EventGroup : FortAthenaLivingWorldEvent::AllEvents)
    {
        if (EventGroup.Event->ActivationGamePhaseStep == GamePhaseStep)
        {
            FortAthenaLivingWorldEvent::ComputeRuntimeEvent(EventGroup.Event, EventGroup.EventType, nullptr);
        }
    }
    HandleGamePhaseStepChangedOG(Object, Frame);
}
void FortAthenaLivingWorldManager::Hook()
{
    // og might be needed
    UHook* Hook = new UHook();

    Hook->Class = AFortAthenaLivingWorldPointProvidersEnabler::StaticClass();
    Hook->Detour = NeedsLoadForClient;
    Hook->Address = 0x108 / 8;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);


    Hook->Class = UFortAthenaLivingWorldEventData::StaticClass();
    Hook->Detour = NeedsLoadForClient;
    Hook->Address = 0x108 / 8;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);


    Hook->Path = "/Script/LagerRuntime.FortAthenaLivingWorldManager.TrySpawnEvent";
    Hook->Detour = TrySpawnEvent;
    Hook->Detour = (void**)&TrySpawnEventOG;
    UKismetHookingLibrary::Hook(Hook, Exec);

    Hook->Path = "/Script/LagerRuntime.FortAthenaLivingWorldManager.RequestEventGeneration";
    Hook->Detour = RequestEventGeneration;
    Hook->Original = (void**)&RequestEventGenerationOG;
    UKismetHookingLibrary::Hook(Hook, Exec); 
    
    Hook->Path = "/Script/FortniteGame.FortGameStateComponent_BattleRoyaleGamePhaseLogic.HandleGamePhaseStepChanged";
    Hook->Detour = HandleGamePhaseStepChanged;
    Hook->Original = (void**)&HandleGamePhaseStepChangedOG;
    UKismetHookingLibrary::Hook(Hook, Exec);
}


//  okay so all offset of lwm stuff is gonna be here to keep track of what was checked 
// sub_B680704(__int64 FFortAthenaLivingWorldDensityGrid, __int64 InActor, double DensityValue, double DensityRange) // density grid addactor // need to init the grid to make it spawn
// 0xB687AA0 getallcategories


//execs to be tested
// OnWorldFinishedInitialization // ai bots stuff
// RequestEventGeneration // no idea
// TrySpawnEvent
// TrySpawnEventForRegistry
// TrySpawnEventWithPreDespawn
// TrySpawnEventWithPreDespawnForRegistry
// HasPendingSpawnRequestForEncounter
// IsEncounterCategoryActive

// B69296C registerspawnedevent


