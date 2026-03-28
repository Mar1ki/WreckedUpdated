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

TArray<FFortAthenaLivingWorldEvent*> FortAthenaLivingWorldEvent::GetAllWorldEventsFromTable(UDataTable* CategoryDataTable)
{
    TArray<FFortAthenaLivingWorldEvent*> AllEvents;
    
    if (!CategoryDataTable)
    {
        UE_LOG(LogLivingWorldManager, Warning, "GetAllWorldEventsFromTable: No DataTable provided");
        return AllEvents;
    }

    auto& RowMap = CategoryDataTable->RowMap;
    for (auto& Row : RowMap)
    {
        auto* RowData = Row.Value();
        if (!RowData) continue;

        auto* CategoryRow = (FFortAthenaLivingWorldCategory*)RowData;

        if (auto* EventTable = CategoryRow->EventTable.Table.Get())
        {
            auto& EventRowMap = EventTable->RowMap;
            for (auto& EventRow : EventRowMap)
            {
                auto* EventRowData = EventRow.Value();
                if (!EventRowData) continue;

                auto* Event = (FFortAthenaLivingWorldEvent*)EventRowData;
                std::cout << "adding event:  " << Event->EventDatas[0].EventData.Get()->Name.ToString().c_str() << std::endl;
                AllEvents.Add(Event);
            }
        }
    }
    UE_LOG(LogLivingWorldManager, Warning, "Total Events Found: %d", AllEvents.Num());
    return AllEvents;
}
// second refactor
FGameplayTag FortAthenaLivingWorldEvent::GetSpawnerTagFromEvent(FFortAthenaLivingWorldEvent* InEvent)
{
    auto EventData = InEvent->EventDatas[0].EventData.Get();
    
    auto TagNum = EventData->ProviderFiltersTagQuery.TagDictionary.Num();

    if (TagNum == 0)
        return FGameplayTag();
    // we will do a scuffed way and just a check for only challenges these ch5 s3 ones
    if (EventData->ProviderFiltersTagQuery.TagDictionary[0].TagName.ToString().contains("Athena.AI.SpawnLocation.Challenge"))
    {
       return EventData->ActorDescriptions[0].SpawnerDataTagQuery.TagDictionary[0];
    }

    if (EventData->ProviderFiltersTagQuery.TagDictionary[0].TagName.ToString().contains("Athena.Vehicle.SpawnLocation.AlwaysSpawn") || EventData->ProviderFiltersTagQuery.TagDictionary[0].TagName.ToString().contains(".Base"))
    {
        if (TagNum < 2)
        {
            UE_LOG(LogLivingWorldManager, Warning, "GetSpawnerTagFromEvent: Returning %s", EventData->ProviderFiltersTagQuery.TagDictionary[0].TagName.ToString().c_str());
            return EventData->ProviderFiltersTagQuery.TagDictionary[0];
        }

        UE_LOG(LogLivingWorldManager, Warning, "GetSpawnerTagFromEvent: Returning %s", EventData->ProviderFiltersTagQuery.TagDictionary[1].TagName.ToString().c_str());
        return EventData->ProviderFiltersTagQuery.TagDictionary[1];
    }

    UE_LOG(LogLivingWorldManager, Warning, "GetSpawnerTagFromEvent: Returning %s", EventData->ProviderFiltersTagQuery.TagDictionary[0].TagName.ToString().c_str());
    return EventData->ProviderFiltersTagQuery.TagDictionary[0];
}
std::map<std::string, UEAllocatedString> PatchMap;
TArray<FFortLivingWorldEventGroup> FortAthenaLivingWorldEvent::AllEvents;
void FortAthenaLivingWorldEvent::TrySpawnEvents()
{


    auto LivingWorldManager = (UFortAthenaLivingWorldManager*)GetGameState()->GetComponentByClass(UFortAthenaLivingWorldManager::StaticClass()); // ig we can use cached lwm
    auto EventArray = GetAllWorldEvents(LivingWorldManager);


    auto SuvEventTable = StaticLoadObject<UDataTable>("/BasicSUV/LWSpawner/LagerDataTable/Lager_CategoryTable_BasicSUV.Lager_CategoryTable_BasicSUV");
    auto DirtBikeEventTable = StaticLoadObject<UDataTable>("/Dirtbike/LWSpawner/LagerDataTables/Lager_CategoryTable_Motorcycle_Dirtbike.Lager_CategoryTable_Motorcycle_Dirtbike");
    auto BossVehiclesEventTable = StaticLoadObject<UDataTable>("/MoonflaxBossVehicle/DataTables/Lager/Lager_CategoryTable_MoonflaxBossVehicles.Lager_CategoryTable_MoonflaxBossVehicles");


 //   std::cout << ValetEvents2.Num() << std::endl;
    // we will store each event types for now bc i have no better idea to handle them
    // i will try to find a good way to store events better 
    TArray<FFortAthenaLivingWorldEvent*> NpcEvents; // handled rn not good but works
    TArray<FFortAthenaLivingWorldEvent*> ValetEvents;
    TArray<FFortAthenaLivingWorldEvent*> OtherEvents; // ig we can handle random shit here like bosses and more prob wildlife too
    TArray<FFortAthenaLivingWorldEvent*> JobBoardEvents;
    TArray<FFortAthenaLivingWorldEvent*> ChallengeEvents; // these ch5 s3 challenges that u take
    
    ValetEvents.Append(GetAllWorldEventsFromTable(SuvEventTable));
    ValetEvents.Append(GetAllWorldEventsFromTable(DirtBikeEventTable));
    ValetEvents.Append(GetAllWorldEventsFromTable(BossVehiclesEventTable));




    PatchMap.insert({"Athena.AI.SpawnLocation.JobBoard.Vehicle", UEAllocatedString("/JobBoardExperimental/ActorBlueprints/BP_StationProp_JobBoard_Experimental_Vehicle.BP_StationProp_JobBoard_Experimental_Vehicle_C")});
    PatchMap.insert({"Athena.AI.SpawnLocation.JobBoard.Bounty", UEAllocatedString("/JobBoardExperimental/ActorBlueprints/BP_StationProp_JobBoard_Experimental_Bounty.BP_StationProp_JobBoard_Experimental_Bounty_C")});
    PatchMap.insert({"Athena.AI.SpawnLocation.JobBoard.Plunder", UEAllocatedString("/JobBoardExperimental/ActorBlueprints/BP_StationProp_JobBoard_Experimental_Treasure.BP_StationProp_JobBoard_Experimental_Treasure_C")});
    PatchMap.insert({"Athena.AI.SpawnLocation.JobBoard.SupplyDrop", UEAllocatedString("/JobBoardExperimental/ActorBlueprints/BP_StationProp_JobBoard_Experimental_SupplyDropGeneric.BP_StationProp_JobBoard_Experimental_SupplyDropGeneric_C")});

    // These Challenges
    PatchMap.insert({"AISpawnerData.Type.Challenge.01", UEAllocatedString("/ChallengeQuests/ActorBlueprints/BP_StationProp_ChallengeQuests_01.BP_StationProp_ChallengeQuests_01_C")});
    PatchMap.insert({"AISpawnerData.Type.Challenge.02", UEAllocatedString("/ChallengeQuests/ActorBlueprints/BP_StationProp_ChallengeQuests_02.BP_StationProp_ChallengeQuests_02_C")});
    PatchMap.insert({"AISpawnerData.Type.Challenge.03", UEAllocatedString("/ChallengeQuests/ActorBlueprints/BP_StationProp_ChallengeQuests_03.BP_StationProp_ChallengeQuests_03_C")});
    PatchMap.insert({"AISpawnerData.Type.Challenge.04", UEAllocatedString("/ChallengeQuests/ActorBlueprints/BP_StationProp_ChallengeQuests_04.BP_StationProp_ChallengeQuests_04_C")});
    PatchMap.insert({"AISpawnerData.Type.Challenge.05", UEAllocatedString("/ChallengeQuests/ActorBlueprints/BP_StationProp_ChallengeQuests_05.BP_StationProp_ChallengeQuests_05_C")});
    PatchMap.insert({"AISpawnerData.Type.Challenge.06", UEAllocatedString("/ChallengeQuests/ActorBlueprints/BP_StationProp_ChallengeQuests_06.BP_StationProp_ChallengeQuests_06_C")});

    
    // valet
    PatchMap.insert({"Athena.Vehicle.SpawnLocation.Valet.SportsCar.Base", UEAllocatedString("/Valet/SportsCar/Valet_SportsCar_Vehicle.Valet_SportsCar_Vehicle_C")}); // alwaysspawn event
    PatchMap.insert({"Athena.Vehicle.SpawnLocation.Valet.SportsCar.Upgraded", UEAllocatedString("/Valet/SportsCar/Valet_SportsCar_Vehicle_Upgrade.Valet_SportsCar_Vehicle_Upgrade_C")}); // upgraded event
    PatchMap.insert({"Athena.Vehicle.SpawnLocation.Valet.SportsCar", UEAllocatedString("/Valet/SportsCar/Valet_SportsCar_Vehicle.Valet_SportsCar_Vehicle_C")}); // base event
    PatchMap.insert({"Athena.Vehicle.SpawnLocation.Motorcycle.Dirtbike", UEAllocatedString("/Dirtbike/Vehicle/Motorcycle_DirtBike_Vehicle.Motorcycle_DirtBike_Vehicle_C")});
    PatchMap.insert({"Athena.Vehicle.SpawnLocation.Valet.BasicSUV", UEAllocatedString("/BasicSUV/Vehicle/Valet_BasicSUV_Vehicle.Valet_BasicSUV_Vehicle_C")});

    // Boss vehicle events
    PatchMap.insert({"Athena.Vehicle.SpawnLocation.Valet.MoonflaxBossVehicles.MegaToof", UEAllocatedString("/MoonflaxBossVehicle/Gameplay/Vehicles/Valet_BossSUV_Moonflax_Vehicle_POISpawned.Valet_BossSUV_Moonflax_Vehicle_POISpawned_C")});
    PatchMap.insert({"Athena.Vehicle.SpawnLocation.Valet.MoonflaxBossVehicles.PrairieSkip", UEAllocatedString("/MoonflaxBossVehicle/Gameplay/Vehicles/Valet_BossSUV_Moonflax_Vehicle_POISpawned.Valet_BossSUV_Moonflax_Vehicle_POISpawned_C")});
    PatchMap.insert({"Athena.Vehicle.SpawnLocation.Valet.MoonflaxBossVehicles.StrategicSpur", UEAllocatedString("/MoonflaxBossVehicle/Gameplay/Vehicles/Valet_BossSportsCar_Moonflax_Vehicle_StrategicSpur_POISpawned.Valet_BossSportsCar_Moonflax_Vehicle_StrategicSpur_POISpawned_C")});

    
    
    // i need to figure out a better way for that filtering
    for (auto Event : EventArray)
    {
       for (auto EventData :  Event->EventDatas)
       {
           for (auto ActorDesc : EventData.EventData->ActorDescriptions)
           {
              auto SpawnerTag =  ActorDesc.SpawnerDataTagQuery.TagDictionary[0].TagName.ToString(); // these events have multiple tags for example valet

               if (SpawnerTag.contains("Tandem") || SpawnerTag.contains("Specialist")) // tandem is npcs and specialists are some sort of npcs idk 
               {
                //   NpcEvents.Add(Event);
                   AllEvents.Add({Event, ELagerEventType::Tandem});
               }
               else if (SpawnerTag.contains("Valet"))
               {
                   //ValetEvents.Add(Event);
                   AllEvents.Add({Event, ELagerEventType::Valet});
               }
               else if (SpawnerTag.contains("JobBoard"))
               {
                 //  JobBoardEvents.Add(Event);
                   AllEvents.Add({Event, ELagerEventType::Misc});
               } else if (SpawnerTag.contains("Challenge"))
               {
                   //ChallengeEvents.Add(Event);
                   AllEvents.Add({Event, ELagerEventType::Misc});
               }
           }
       }
    }
   /*for (auto JobBoardEvent : JobBoardEvents)
     {
         ComputeRuntimeEvent(JobBoardEvent, ELagerEventType::Misc, nullptr);
     }
    std::cout << "Challenge: " << ChallengeEvents.Num() << std::endl;*/
    

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
  /*  for (auto ChallengeEvent : ChallengeEvents)
    {
        ComputeRuntimeEvent(ChallengeEvent, ELagerEventType::Misc, nullptr);
    }

    for (auto ValetEvent : ValetEvents)
    {
        ComputeRuntimeEvent(ValetEvent, ELagerEventType::Valet, nullptr);
    }*/
}
bool FortAthenaLivingWorldEvent::ComputeRuntimeEvent(FFortAthenaLivingWorldEvent* InEvent, ELagerEventType EventType, UClass* OptionalActorClass = nullptr)
{
    auto SpawnCount = InEvent->MaxCount[0]; // thats so ahh why they use a fucking tarray to hold one value :sob:
    auto SpawnerTag = GetSpawnerTagFromEvent(InEvent);

    if (EventType == ELagerEventType::Valet || EventType == ELagerEventType::Misc)
    {
        auto EventData = InEvent->EventDatas[0].EventData.Get();
        auto PointProviders = FortAthenaLivingWorldStaticPointProvider::GetPointProvidersFromTagQuery(SpawnerTag);
        std::cout << "Found : " << PointProviders.Num()<< " PointProviders" << std::endl;
        UClass* ActorToSpawn = nullptr;
        
        ActorToSpawn = GetClassFromPatchMap(PatchMap, SpawnerTag.TagName);
        if (ActorToSpawn == nullptr)
        {
            UE_LOG(LogLivingWorldManager, Error, "Couldn't Find the actor, please check the patch map %s", SpawnerTag.TagName.ToString().c_str());
            return false;
        }
        
        for (int i = 0; i < SpawnCount; i++)
        {
           auto SelectedPointProvider = (AFortAthenaLivingWorldVehiclePointProvider*)FortAthenaLivingWorldStaticPointProvider::GetRandomPointProviderForEventSpawn(PointProviders);

            if (SelectedPointProvider == nullptr)
            {
                std::cout << "No Point Provider Available";
                return false;
            }
            if (ActorToSpawn)
            {
                SelectedPointProvider->bStartEnabled = false;
                SelectedPointProvider->DisablePointProvider();
                auto SpawnTransform = SelectedPointProvider->GetTransform();
                GetWorld()->SpawnActor(ActorToSpawn, SpawnTransform, nullptr);
                UE_LOG(LogLivingWorldManager, Log, "Successfully spawned a actor with name: %s", ActorToSpawn->Name.ToString().c_str());
            }
        }
        return true;
    }
    else if (EventType == ELagerEventType::Tandem)
    {
        auto idk = GetSpawnerDataListFromTandemEvent(InEvent);
        if (idk == nullptr) return false;
        auto PatrolPath = GetPatrolPathFromEvent(InEvent);
        auto PatrolPoint = PatrolPath->GetPatrolPoint(1);
        ((UAthenaAISystem*)UWorld::GetWorld()->AISystem)->AISpawner->RequestSpawn(idk, PatrolPoint->GetTransform(), false);
          
        return true;
    }
    else
    {
        return false;
    }
}
