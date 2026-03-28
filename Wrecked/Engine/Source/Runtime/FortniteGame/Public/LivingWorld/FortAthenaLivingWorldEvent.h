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
struct FFortLivingWorldEventGroup
{
    FFortAthenaLivingWorldEvent* Event;
    ELagerEventType EventType;
};
namespace FortAthenaLivingWorldEvent
{
    extern TArray<FFortLivingWorldEventGroup> AllEvents;
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

    inline EAthenaGamePhaseStep GetSpawnGamePhaseStepFromEvent(FFortAthenaLivingWorldEvent* Event)
    {
        return Event->ActivationGamePhaseStep;
    }
    inline UFortAthenaAISpawnerDataComponentList* GetSpawnerDataListFromTandemEvent(FFortAthenaLivingWorldEvent* Event)
    {
        auto TandemTag = Event->EventDatas[0].EventData.Get()->ProviderFiltersTagQuery.TagDictionary[0];
        FString LastPart;
        UKismetStringLibrary::Split(UKismetStringLibrary::Conv_NameToString(TandemTag.TagName), TEXT("."), nullptr, &LastPart, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
        auto SpawnerDataFullName = "BP_AIBotSpawnerData_NPC_" + LastPart.ToString() + ".BP_AIBotSpawnerData_NPC_" + LastPart.ToString() + "_C";
        auto Path = "/NPCLibrary/NPCs/" + LastPart.ToString() + "/" + SpawnerDataFullName;
        std::cout << Path << std::endl;
        
        auto Obj = StaticLoadObject<UClass>(Path);
        if (Obj)
        {
            return UFortAthenaAISpawnerData::CreateComponentListFromClass(Obj, UWorld::GetWorld());
        } else
        {
            return nullptr;
        }
    }
    inline AFortAthenaPatrolPath* GetPatrolPathFromEvent(FFortAthenaLivingWorldEvent* InEvent)
    {
        if (!InEvent || InEvent->EventDatas.IsEmpty())
            return nullptr;

        auto EventData = InEvent->EventDatas[0].EventData.Get();
        if (!EventData || EventData->ProviderFiltersTagQuery.TagDictionary.IsEmpty())
            return nullptr;

        auto EventTag = EventData->ProviderFiltersTagQuery.TagDictionary[0];

        auto AllPatrolPaths = GetObjectsOfClass<AFortAthenaPatrolPath>();
        for (auto PatrolPath : AllPatrolPaths)
        {
            if (!PatrolPath || PatrolPath->GameplayTags.GameplayTags.IsEmpty())
                continue;

            auto InputTag = PatrolPath->GameplayTags.GameplayTags[0];
            if (UBlueprintGameplayTagLibrary::EqualEqual_GameplayTag(EventTag, InputTag))
                return PatrolPath;
        }

        return nullptr;
    }
    TArray<FFortAthenaLivingWorldEvent*> GetAllWorldEvents(UFortAthenaLivingWorldManager* LivingWorldManager);
    TArray<FFortAthenaLivingWorldEvent*> GetAllWorldEventsFromTable(UDataTable* CategoryDataTable);

    FGameplayTag GetSpawnerTagFromEvent(FFortAthenaLivingWorldEvent* InEvent);
    bool ComputeRuntimeEvent(FFortAthenaLivingWorldEvent* InEvent, ELagerEventType EventType, UClass*);
    void TrySpawnEvents();
    void Hook();
}
