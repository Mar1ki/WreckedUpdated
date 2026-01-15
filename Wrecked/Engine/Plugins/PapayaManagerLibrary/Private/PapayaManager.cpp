#include "pch.h"
#include "PapayaManager.h"

#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"

inline float EvaluateScalableFloat(FScalableFloat& Float, float Level)
{
    if (Float.Curve.CurveTable)
    {
        float Result;
        UDataTableFunctionLibrary::EvaluateCurveTableRow(Float.Curve.CurveTable, Float.Curve.RowName, Level, nullptr, &Result, FString());
        return Result;
    }
    
    return Float.Value;
}

void PapayaManager::test()
{
    auto GrapplerMachine = StaticLoadObject<UClass>("/PapayaGameplay/Items/VendingMachine/B_Papaya_VendingMachine_Grappler.B_Papaya_VendingMachine_Grappler_C");
    
    TArray<AActor*> machines;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), GrapplerMachine, &machines);

    for (auto machine : machines)
    {
        auto vendingmachine = (ABuildingItemCollectorActor*)machine;
       std::cout << "RowName: " << vendingmachine->ItemCollectorHandle.RowName.ToString().c_str() << std::endl;

        auto It =  vendingmachine->ItemCollectorHandle.DataTable->RowMap.Find(vendingmachine->ItemCollectorHandle.RowName, [](const FName& Left, const FName& Right) {return Left == Right; });
        if (It.IsValid()) {
            auto Pair =  vendingmachine->ItemCollectorHandle.DataTable->RowMap[It.GetIndex()];
            FItemCollectorRow* Value = (FItemCollectorRow*)Pair.Second;
          std::cout << "datatable: "<<  Value->OverrideItemsTable->GetName().c_str() << std::endl;
            
            for (auto item : Value->OverrideItemsTable->RowMap)
            {
                auto* RowData = (FItemCollectorOverrideItemRow*)(item.Value());
    
                if (RowData)
                {

                    for (auto collection : vendingmachine->ItemCollections)
                    {
                        collection.OutputItemEntry.Free();
                        collection.OutputItem = nullptr;
                        auto WID = StaticFindObject<UFortWorldItemDefinition>(RowData->ItemPrimaryAssetId.PrimaryAssetName.ToString().c_str(),  nullptr, (UObject*)-1);
                        collection.OutputItem = WID;
                        collection.bUseDefinedOutputItem = true;
                    }
                    std::cout << "Asset Name: " << RowData->ItemPrimaryAssetId.PrimaryAssetName.ToString().c_str() << std::endl;
                    std::cout << "Quantity: " << RowData->Quantity << std::endl;
                }
            }
            
        }
    }
    
}
