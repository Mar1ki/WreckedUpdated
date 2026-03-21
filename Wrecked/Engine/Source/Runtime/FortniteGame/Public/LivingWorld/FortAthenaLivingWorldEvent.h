#pragma once
#include "pch.h"
#include "SDK/SDK/GameplayTags_classes.hpp"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"

enum class ELagerEventType : uint8
{
    Unknown                                  = 0,
    Valet                                    = 1,
    Static                                   = 2,
    Tandem                                   = 3,
    AIMisc                                   = 4,
    Boss                                     = 5,
    Misc                                     = 6,
    ELagerEventType_MAX                      = 7,
};
namespace FortAthenaLivingWorldEvent
{
    inline UClass* GetClassFromPatchMap(const std::map<std::string, UEAllocatedString>& PatchMap, const FName& TagName)
    {
        auto Pair = PatchMap.find(TagName.ToString().c_str());
        if (Pair == PatchMap.end()) return nullptr;
        return StaticLoadObject<UClass>(Pair->second.c_str());
    }
    inline UFortAthenaLivingWorldEventData* GetEventData(FFortAthenaLivingWorldEvent* Event)
    {
        return Event->EventDatas[0].EventData.Get();
    }

    TArray<FFortAthenaLivingWorldEvent*> GetAllWorldEvents(UFortAthenaLivingWorldManager* LivingWorldManager);
    TArray<FFortAthenaLivingWorldEvent*> GetAllWorldEventsFromTable(UDataTable* CategoryDataTable);

    TArray<AFortAthenaPatrolPath*> GetPatrolPathsFromTagQuery(const FGameplayTag& SearchTag);
    FGameplayTag GetSpawnerTagFromEvent(FFortAthenaLivingWorldEvent* InEvent);
    bool ComputeRuntimeEvent(FFortAthenaLivingWorldEvent* InEvent, ELagerEventType EventType, UClass*);
    void TrySpawnEvents();
    void Hook();
}
