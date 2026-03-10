#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/LivingWorld/FortAthenaLivingWorldEvent.h"

#include "Engine/Source/Runtime/FortniteGame/Public/LivingWorld/FortAthenaLivingWorldStaticPointProvider.h"


TArray<FFortAthenaLivingWorldEvent*> FortAthenaLivingWorldEvent::GetAllWorldEvents(UFortAthenaLivingWorldManager* LivingWorldManager)
{
    std::cout << "GetAllWorldEvents" << std::endl;
    auto WorldConfig = LivingWorldManager->GetConfig();
    TArray<FFortAthenaLivingWorldEvent*> AllEvents;
    for (auto& LivingWorldCategory : WorldConfig->CategoryTableDatas)
    {
        auto* CategoryDataTable = LivingWorldCategory.CategoryTable.Get();
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
    UE_LOG(LogLivingWorldManager, Warning, "Total Events Found: %d", AllEvents.Num());
    return AllEvents;
}

TArray<FFortAthenaLivingWorldEvent*> GetAllWorldEvents(UFortAthenaLivingWorldManager* LivingWorldManager)
{
    std::cout << "GetAllWorldEvents" << std::endl;
    auto WorldConfig = LivingWorldManager->GetConfig();
    TArray<FFortAthenaLivingWorldEvent*> AllEvents;
    for (auto& LivingWorldCategory : WorldConfig->CategoryTableDatas)
    {
        auto* CategoryDataTable = LivingWorldCategory.CategoryTable.Get();
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
    UE_LOG(LogLivingWorldManager, Warning, "Total Events Found: %d", AllEvents.Num());
    return AllEvents;
}


FGameplayTag FixTandemTag(const FGameplayTag& Tag)
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

TArray<AFortAthenaPatrolPath*> FortAthenaLivingWorldEvent::GetPatrolPathsFromTagQuery(const FGameplayTag& SearchTag)
{
    auto SearchTagFixed = FixTandemTag(SearchTag);
    TArray<AFortAthenaPatrolPath*> FoundPoints;
    
    for (auto PatrolPath : GetObjectsOfClass<AFortAthenaPatrolPath>())
    {
        for (auto tag : PatrolPath->GameplayTags.GameplayTags)
        {
            auto PatrolTagFixed = FixTandemTag(FGameplayTag{tag.TagName});
            
            if (PatrolTagFixed == SearchTagFixed)
            {
                FoundPoints.Add(PatrolPath);
                break;
            }
        }
    }
    return FoundPoints;
}


FGameplayTag FortAthenaLivingWorldEvent::GetSpawnerTagFromEvent(FFortAthenaLivingWorldEvent* InEvent)
{
    auto EventData =  InEvent->EventDatas[0].EventData.Get();

    if (EventData->ProviderFiltersTagQuery.TagDictionary[0].TagName.ToString().contains("Athena.Vehicle.SpawnLocation.AlwaysSpawn"))
    {
        UE_LOG(LogLivingWorldManager, Warning, "Found a scuffed always spawn event using the second tag");
        return EventData->ProviderFiltersTagQuery.TagDictionary[1];
    } else
    {
        return EventData->ProviderFiltersTagQuery.TagDictionary[0]; // new way returns Athena.AI.SpawnLocation.JobBoard.Vehicle // which point providers use :D
  
    }
    //return NpcEvent->EventDatas[0].EventData->ActorDescriptions[0].SpawnerDataTagQuery.TagDictionary[0].TagName.ToString(); // old way it returned AISpawnerData.Type.JobBoard.Vehicle 
}

std::map<std::string, UEAllocatedString> PatchMap;
void FortAthenaLivingWorldEvent::TrySpawnEvents()
{
    auto LivingWorldManager = (UFortAthenaLivingWorldManager*)GetGameState()->GetComponentByClass(UFortAthenaLivingWorldManager::StaticClass()); // ig we can use cached lwm
    auto EventArray = GetAllWorldEvents(LivingWorldManager);

    // we will store each event types for now bc i have no better idea to handle them
    // i will try to find a good way to store events better 
    TArray<FFortAthenaLivingWorldEvent*> NpcEvents; // handled rn not good but works
    TArray<FFortAthenaLivingWorldEvent*> ValetEvents;
    TArray<FFortAthenaLivingWorldEvent*> OtherEvents; // ig we can handle random shit here like bosses and more prob wildlife too
    TArray<FFortAthenaLivingWorldEvent*> JobBoardEvents;
    TArray<FFortAthenaLivingWorldEvent*> ChallengeEvents; // these ch5 s3 challenges that u take 


    PatchMap.insert({"Athena.AI.SpawnLocation.JobBoard.Vehicle", UEAllocatedString("/JobBoardExperimental/ActorBlueprints/BP_StationProp_JobBoard_Experimental_Vehicle.BP_StationProp_JobBoard_Experimental_Vehicle_C")});
    PatchMap.insert({"Athena.AI.SpawnLocation.JobBoard.Bounty", UEAllocatedString("/JobBoardExperimental/ActorBlueprints/BP_StationProp_JobBoard_Experimental_Bounty.BP_StationProp_JobBoard_Experimental_Bounty_C")});
    PatchMap.insert({"Athena.AI.SpawnLocation.JobBoard.Plunder", UEAllocatedString("/JobBoardExperimental/ActorBlueprints/BP_StationProp_JobBoard_Experimental_Treasure.BP_StationProp_JobBoard_Experimental_Treasure_C")});
    PatchMap.insert({"Athena.AI.SpawnLocation.JobBoard.SupplyDrop", UEAllocatedString("/JobBoardExperimental/ActorBlueprints/BP_StationProp_JobBoard_Experimental_SupplyDropGeneric.BP_StationProp_JobBoard_Experimental_SupplyDropGeneric_C")});

    // These Challenges
    PatchMap.insert({"Athena.AI.SpawnLocation.Challenge", UEAllocatedString("/JobBoardExperimental/ActorBlueprints/BP_StationProp_JobBoard_Experimental_SupplyDropGeneric.BP_StationProp_JobBoard_Experimental_SupplyDropGeneric_C")});


    
    for (auto Event : EventArray)
    {
       for (auto EventData :  Event->EventDatas)
       {
           for (auto ActorDesc : EventData.EventData->ActorDescriptions)
           {
              auto SpawnerTag =  ActorDesc.SpawnerDataTagQuery.TagDictionary[0].TagName.ToString(); // these events have multiple tags for example valet

               if (SpawnerTag.contains("Tandem") || SpawnerTag.contains("Specialist")) // tandem is npcs and specialists are some sort of npcs idk 
               {
                   NpcEvents.Add(Event);
               }
               else if (SpawnerTag.contains("Valet"))
               {
                   ValetEvents.Add(Event);
               }
               else if (SpawnerTag.contains("JobBoard"))
               {
                   JobBoardEvents.Add(Event);
               }
           }
       }
    }
    
    for (auto JobBoardEvent : JobBoardEvents)
    {
        auto SpawnerTag = GetSpawnerTagFromEvent(JobBoardEvent);
        auto MaxCount = JobBoardEvent->MaxCount[0];

        auto resultig = PatchMap.find(SpawnerTag.TagName.ToString().c_str());
        if (resultig == PatchMap.end())
            continue;

        auto Path = resultig->second;
        auto Actor = StaticLoadObject<UClass>(Path);
        if (!Actor) continue;
        

        auto Out = FortAthenaLivingWorldStaticPointProvider::GetPointProvidersFromTagQuery(SpawnerTag);
        if (Out.Num() == 0) continue;

        int SpawnedCount = 0;
        for (int i = 0; i < Out.Num() && SpawnedCount < MaxCount; i++)
        {
            auto SpawnProvider = Out[i];
            auto SpawnLoc = SpawnProvider->GetTransform();
            
            GetWorld()->SpawnActor(Actor, SpawnLoc);
            SpawnProvider->bStartEnabled = false;
            SpawnedCount++;
        }
    }
    std::cout << "Challenge: " << ChallengeEvents.Num() << std::endl;
    

    /*for (auto NpcEvent : NpcEvents)
    {
        for (auto EventData : NpcEvent->EventDatas)
        {
           for (auto ActorDesc : EventData.EventData->ActorDescriptions)
           {
               // if u have a better way for making this whole ass path then do it
               auto SpawnerTagFull = ActorDesc.SpawnerDataTagQuery.TagDictionary[0].TagName.ToString();
               size_t lastDot = SpawnerTagFull.find_last_of('.');
               UEAllocatedString npcName = (lastDot != UEAllocatedString::npos) ? SpawnerTagFull.substr(lastDot + 1) : SpawnerTagFull;

               UEAllocatedString result = "/NPCLibrary/NPCs/" + npcName + "/BP_AIBotSpawnerData_NPC_" + npcName + ".BP_AIBotSpawnerData_NPC_" + npcName + "_C";
               auto SpawnerClass = StaticLoadObject<UClass>(result);

               if (SpawnerClass)
               {
                   auto ComponentList = UFortAthenaAISpawnerData::CreateComponentListFromClass(SpawnerClass, UWorld::GetWorld());
                   auto PatrolPaths =  GetPatrolPathsFromTagQuery(ActorDesc.SpawnerDataTagQuery.TagDictionary[0]); // some npcs have more paths we will need to test it
                   auto RandomPath = PatrolPaths[rand() % PatrolPaths.Num()];
                   auto RandomPatrolPoint = RandomPath->PatrolPoints[rand() % RandomPath->PatrolPoints.Num()];
                   ((UAthenaAISystem*)UWorld::GetWorld()->AISystem)->AISpawner->RequestSpawn(ComponentList, RandomPatrolPoint->GetTransform(), true);
                   
                   // auto Loc = RandomPatrolPoint->K2_GetActorLocation();
                   //std::cout << "Spawned at: X=" << Loc.X << " Y=" << Loc.Y << " Z=" << Loc.Z << std::endl;
               }
           }
        } 
    } */

    for (auto ValetEvent : ValetEvents)
    {
        auto EventData = ValetEvent->EventDatas[0].EventData.Get();
        auto Tag = GetSpawnerTagFromEvent(ValetEvent);

        
      //  FortAthenaLivingWorldStaticPointProvider::GetPointProvidersFromTagQuery();

     //   FortAthenaLivingWorldStaticPointProvider::GetPointProvidersFromTagQuery();

       // std::cout << "EventName: " << EventData->Name.ToString().c_str() << "EventTag: " << Tag.c_str() << std::endl;

    }
}
