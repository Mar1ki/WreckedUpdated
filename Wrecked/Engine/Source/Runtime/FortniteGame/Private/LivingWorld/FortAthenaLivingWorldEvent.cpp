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

                        for (auto EventDatas : Event->EventDatas)
                        {
                            for (auto idk : EventDatas.EventData.Get()->ActorDescriptions)
                            {
                               std::cout << idk.ActorClass->Name.ToString().c_str() << std::endl;
                            }
                            
                        }
                        AllEvents.Add(Event);
                    }
                }
            }
        }
    }
    UE_LOG(LogLivingWorldManager, Warning, "Total Events Found: %d", AllEvents.Num()); // disable that if u want but rn i need it bc config loves to be sometimes empty when calling early
    return AllEvents;
}

TArray<AFortAthenaLivingWorldStaticPointProvider*> GetPointProviderFromTagQuery(const FGameplayTag& SearchTag)
{
    TArray<AFortAthenaLivingWorldStaticPointProvider*> FoundPoints;
    auto LivingWorldManager = (UFortAthenaLivingWorldManager*)GetGameState()->GetComponentByClass(UFortAthenaLivingWorldManager::StaticClass());
    
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


