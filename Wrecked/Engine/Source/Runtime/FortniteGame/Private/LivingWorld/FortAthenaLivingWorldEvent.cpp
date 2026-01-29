#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/LivingWorld/FortAthenaLivingWorldEvent.h"


TArray<FFortAthenaLivingWorldEvent*> FortAthenaLivingWorldEvent::GetAllWorldEvents(UFortAthenaLivingWorldManager* LivingWorldManager)
{
    std::cout << "GetAllWorldEvents" << std::endl;
    auto WorldConfig = LivingWorldManager->GetConfig(); // ik we can get the default config by just lwm->DefaultLagerConfig but it doesnt load the cfg sometimes idk why
    TArray<FFortAthenaLivingWorldEvent*> AllEvents;
    for (auto& LivingWorldCategory : WorldConfig->CategoryTableDatas)
    {
         //UE_LOG(LogLivingWorldManager, Log, "CategoryTableName: %s", LivingWorldCategory.Name.ToString().c_str());
       //  UE_LOG(LogLivingWorldManager, Log, "IsEnabled: %s", std::to_string(LivingWorldCategory.IsEnabled.Value).c_str());
        
        auto* CategoryDataTable = LivingWorldCategory.CategoryTable.Get();
        // idk it works but for me it feels like its unproper
        if (CategoryDataTable)
        {
            auto& RowMap = CategoryDataTable->RowMap;
            for (auto& Row : RowMap)
            {
                auto* RowData = Row.Value();
                auto* CategoryRow = (FFortAthenaLivingWorldCategory*)RowData;
                
                if (auto* EventTable = CategoryRow->EventTable.Table.Get())
                {
                    auto& EventRowMap = EventTable->RowMap;
                    for (auto& EventRow : EventRowMap)
                    {
                        auto* EventRowData = EventRow.Value();
                        auto* Event = (FFortAthenaLivingWorldEvent*)EventRowData;
                        AllEvents.Add(Event);
                    }
                }
            }
        }
    }
    UE_LOG(LogLivingWorldManager, Warning, "Total Events Found: %d", AllEvents.Num()); // disable that if u want but rn i need it bc config loves to be sometimes empty when calling early
    return AllEvents;
}

TArray<AFortAthenaLivingWorldStaticPointProvider*> GetPointProvidersFromTagQuery(const FGameplayTag& SearchTag)
{
    TArray<AFortAthenaLivingWorldStaticPointProvider*> FoundPoints;
    auto LivingWorldManager = (UFortAthenaLivingWorldManager*)GetGameState()->GetComponentByClass(UFortAthenaLivingWorldManager::StaticClass()); // ig we can use cached lwm
    for (auto Point : LivingWorldManager->PointProviders)
    {
        auto PointProvider = (AFortAthenaLivingWorldStaticPointProvider*)Point.ObjectPointer;
        if (PointProvider->FiltersTags.HasTag(SearchTag))
        {
            FoundPoints.Add(PointProvider);
        }
    }
    return FoundPoints;
}
FGameplayTag FixAssString(const FGameplayTag& Tag)
{
    UEAllocatedString TagStr = Tag.TagName.ToString();
    size_t lastDot = TagStr.rfind('.');
    if (lastDot != UEAllocatedString::npos)
    {
        UEAllocatedString temp = TagStr.substr(0, lastDot);
        size_t secondLastDot = temp.rfind('.');
        
        if (secondLastDot != UEAllocatedString::npos)
        {
            FGameplayTag Result;
            Result.TagName = UKismetStringLibrary::Conv_StringToName(TagStr.substr(secondLastDot + 1));
            return Result;
        }
    }
    return Tag;
}

// yes we will create a diff func for finding tandem spawners bc im a lazy ass to make one good func
TArray<AFortAthenaPatrolPath*> FortAthenaLivingWorldEvent::GetPatrolPathsFromTagQuery(const FGameplayTag& SearchTag)
{

    auto SearchTagFixed = FixAssString(SearchTag);
    TArray<AFortAthenaPatrolPath*> FoundPoints;
    
    for (auto PatrolPath : GetObjectsOfClass<AFortAthenaPatrolPath>())
    {
        for (auto tag : PatrolPath->GameplayTags.GameplayTags)
        {
            auto PatrolTagFixed = FixAssString(FGameplayTag{tag.TagName});
            
            if (PatrolTagFixed == SearchTagFixed)
            {
                FoundPoints.Add(PatrolPath);
                break;
            }
        }
    }
    return FoundPoints;
}


void FortAthenaLivingWorldEvent::TrySpawnEvents()
{
    auto LivingWorldManager = (UFortAthenaLivingWorldManager*)GetGameState()->GetComponentByClass(UFortAthenaLivingWorldManager::StaticClass()); // ig we can use cached lwm
    auto EventArray = GetAllWorldEvents(LivingWorldManager);

    // we will store each event types for now bc i have no better idea to handle them
    // i will try to find a good way to store events better 
    TArray<FFortAthenaLivingWorldEvent*> NpcEvents;
    TArray<FFortAthenaLivingWorldEvent*> ValetEvents;
    TArray<FFortAthenaLivingWorldEvent*> OtherEvents; // ig we can handle random shit here like bosses and more prob wildlife too


    for (auto Event : EventArray)
    {
       for (auto EventData :  Event->EventDatas)
       {
           for (auto ActorDesc : EventData.EventData->ActorDescriptions)
           {
              auto SpawnerTag =  ActorDesc.SpawnerDataTagQuery.TagDictionary[0].TagName.ToString();

               if (SpawnerTag.contains("Tandem") || SpawnerTag.contains("Specialist")) // tandem is normal npcs and specialists are specialist npcs
               {
                   NpcEvents.Add(Event);
               }
               else if (SpawnerTag.contains("Valet"))
               {
                   ValetEvents.Add(Event);
               }
               else
               {
                   OtherEvents.Add(Event);
               }
           }
       }
    }


    for (auto NpcEvent : NpcEvents)
    {
        for (auto EventData : NpcEvent->EventDatas)
        {
           for (auto ActorDesc : EventData.EventData->ActorDescriptions)
           {
               auto SpawnerTagFull = ActorDesc.SpawnerDataTagQuery.TagDictionary[0].TagName.ToString();
               size_t lastDot = SpawnerTagFull.find_last_of('.');
               UEAllocatedString npcName = (lastDot != UEAllocatedString::npos) ? SpawnerTagFull.substr(lastDot + 1) : SpawnerTagFull;

               UEAllocatedString result = "/NPCLibrary/NPCs/" + npcName + "/BP_AIBotSpawnerData_NPC_" + npcName + ".BP_AIBotSpawnerData_NPC_" + npcName + "_C";
               auto Class = StaticLoadObject<UClass>(result);

               if (Class)
               {
                   auto ComponentList = UFortAthenaAISpawnerData::CreateComponentListFromClass(Class, UWorld::GetWorld());
                   auto PatrolPaths =  GetPatrolPathsFromTagQuery(ActorDesc.SpawnerDataTagQuery.TagDictionary[0]); // not proper as hell bc some shit has multiple tags

                   auto RandomPath = PatrolPaths[rand() % PatrolPaths.Num()];

                   auto RandomPatrolPoint = RandomPath->PatrolPoints[rand() % RandomPath->PatrolPoints.Num()];
                   ((UAthenaAISystem*)UWorld::GetWorld()->AISystem)->AISpawner->RequestSpawn(ComponentList, RandomPatrolPoint->GetTransform(), true);
                  // auto Loc = RandomPatrolPoint->K2_GetActorLocation();
                   //std::cout << "Spawned at: X=" << Loc.X << " Y=" << Loc.Y << " Z=" << Loc.Z << std::endl;
               }
           }
        }
    } 
    
}

