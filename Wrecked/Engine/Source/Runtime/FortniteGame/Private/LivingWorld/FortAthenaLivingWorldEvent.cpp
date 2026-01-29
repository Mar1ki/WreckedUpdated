#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/LivingWorld/FortAthenaLivingWorldEvent.h"


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

FGameplayTag FixTandemTag(const FGameplayTag& Tag)
{
    auto TagFStr = Tag.TagName.ToString();
    
    int32 LastDot = UKismetStringLibrary::FindSubstring(TagFStr, L".", false, true, 0);
    
    if (LastDot != -1)
    {
        FString Temp = UKismetStringLibrary::Left(TagFStr, LastDot);
        
        int32 SecondLastDot = UKismetStringLibrary::FindSubstring(Temp, L".", false, true, 0);
        
        if (SecondLastDot != -1)
        {
            FString NewTagString = UKismetStringLibrary::RightChop(TagFStr, SecondLastDot + 1);
            FGameplayTag Result;
            Result.TagName = UKismetStringLibrary::Conv_StringToName(NewTagString);
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


void FortAthenaLivingWorldEvent::TrySpawnEvents()
{
    auto LivingWorldManager = (UFortAthenaLivingWorldManager*)GetGameState()->GetComponentByClass(UFortAthenaLivingWorldManager::StaticClass()); // ig we can use cached lwm
    auto EventArray = GetAllWorldEvents(LivingWorldManager);

    // we will store each event types for now bc i have no better idea to handle them
    // i will try to find a good way to store events better 
    TArray<FFortAthenaLivingWorldEvent*> NpcEvents; // handled rn not good but works
    TArray<FFortAthenaLivingWorldEvent*> ValetEvents;
    TArray<FFortAthenaLivingWorldEvent*> OtherEvents; // ig we can handle random shit here like bosses and more prob wildlife too


    for (auto Event : EventArray)
    {
       for (auto EventData :  Event->EventDatas)
       {
           for (auto ActorDesc : EventData.EventData->ActorDescriptions)
           {
              auto SpawnerTag =  ActorDesc.SpawnerDataTagQuery.TagDictionary[0].TagName.ToString(); // these events have multiple tags for example valet

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
    } 
    
}

