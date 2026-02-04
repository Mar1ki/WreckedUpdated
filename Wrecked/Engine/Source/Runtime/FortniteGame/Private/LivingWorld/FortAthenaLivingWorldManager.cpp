#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/LivingWorld/FortAthenaLivingWorldManager.h"
// lot of if statements and for so yh refactor that shit later
void FortAthenaLivingWorldManager::PerformPointProviderSpawn()
{
    auto GameState = GetGameState();
    auto LWM = (UFortAthenaLivingWorldManager*)GameState->GetComponentByClass(UFortAthenaLivingWorldManager::StaticClass());

    for (auto Point : LWM->PointProviders)
    {
        auto PointProvider = (AFortAthenaLivingWorldStaticPointProvider*)Point.ObjectPointer;

        if (PointProvider)
        {
            bool hasVehicleTag = false;

            for (auto& GTag : PointProvider->FiltersTags.GameplayTags)
            {
                // Most bitchy way to find vehicles but works 
                if (GTag.TagName.GetRawString().contains("Athena.Vehicle"))
                {
                    hasVehicleTag = true;
                    break;
                }
            }
            if (hasVehicleTag)
            {
                auto New = (AFortAthenaLivingWorldVehiclePointProvider*)PointProvider;
                for (auto& GTag : New->ForceMods.GameplayTags)
                {
                    std::cout << GTag.TagName.GetRawString().c_str() << " ";
                }
                std::cout << "Point: " << PointProvider->GetName().c_str() << std::endl;
                std::cout << "Vehicle Tags: ";
                for (auto& GTag : PointProvider->FiltersTags.GameplayTags)
                {
                    std::cout << GTag.TagName.GetRawString().c_str() << " ";
                }
                std::cout << std::endl << std::endl;
            }
            if (PointProvider->Name.ToString().contains("JobBoard"))
            {
                std::cout << "found shit" << std::endl;
                static auto BGAClass = StaticFindObject<UClass>("/Script/Engine.BlueprintGeneratedClass");
                static auto VehicleClass = StaticLoadObject<UClass>("/JobBoardExperimental/ActorBlueprints/BP_StationProp_JobBoard_Experimental_Bounty.BP_StationProp_JobBoard_Experimental_Bounty_C", BGAClass);
                auto Location = PointProvider->K2_GetActorLocation();
                auto Rotation = PointProvider->K2_GetActorRotation();
                auto SpawnedVehicle = GetWorld()->SpawnActor(VehicleClass, Location, Rotation);
            }
        }
    }
}

inline void logtofile2(const std::string& message) {
    std::ofstream file("log.txt", std::ios::app);
    if (file.is_open()) {
        file << message << std::endl;
    }
}

char (*DensityGridAddActorOG)(FFortAthenaLivingWorldDensityGrid*, AActor*, double, double);
char DensityGridAddActor(FFortAthenaLivingWorldDensityGrid* DensityGrid, AActor* InActor, double DensityValue, double DensityRange)
{

    std::cout << InActor->Name.ToString().c_str() << std::endl;




    return DensityGridAddActorOG(DensityGrid, InActor, DensityValue, DensityRange);
}

static bool NeedsLoadForClient() {
    return true;
}

void FortAthenaLivingWorldManager::Hook()
{
    // og might be needed
    UHook* Hook = new UHook();

    Hook->Detour = DensityGridAddActor;
    Hook->Address = ImageBase + 0xB680704;
    Hook->Original = (void**)&DensityGridAddActorOG;
    UKismetHookingLibrary::Hook(Hook, Address);


    Hook->Class = AFortAthenaLivingWorldPointProvidersEnabler::StaticClass();
    Hook->Detour = NeedsLoadForClient;
    Hook->Address = 0x108 / 8;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);


    Hook->Class = UFortAthenaLivingWorldEventData::StaticClass();
    Hook->Detour = NeedsLoadForClient;
    Hook->Address = 0x108 / 8;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);

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


