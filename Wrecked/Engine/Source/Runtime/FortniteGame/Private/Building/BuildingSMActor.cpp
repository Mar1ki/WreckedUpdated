#include "pch.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Building/BuildingSMActor.h"

#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Kismet/FortKismetLibrary.h"


bool BuildingSMActor::AttemptSpawnResources(ABuildingSMActor* BuildingActor, FBuildingSMActorAttemptSpawnResourcesParams* Params)
{
	
	auto DamageDealt = Params->ActualDamageDealt;
	auto WeakSpot = Params->bJustHitWeakspot;
	auto Pawn = Params->InstigatorPawn.Get();
    auto PlayerController = (AFortPlayerControllerAthena*)Pawn->Controller;
	if (PlayerController->IsA(AFortAthenaAIBotController::StaticClass())) return false;
    static auto CurveTable = StaticLoadObject<UCurveTable>("/Game/Balance/DataTables/ResourceRates.ResourceRates");
	auto ClassData = GetSparseData<FBuildingSMActorClassData>(BuildingActor->Class);
	
	if (!CurveTable)
		return false;
	
    float Average = 1.f;
    EEvaluateCurveTableResult Result = {};
    UDataTableFunctionLibrary::EvaluateCurveTableRow(CurveTable, ClassData->BuildingResourceAmountOverride.RowName, 0.f, &Result, &Average, FString());
	float FinalResourceCount = round(Average / (BuildingActor->GetMaxHealth() / DamageDealt));
	if (FinalResourceCount > 0)
	{
		PlayerController->ClientReportDamagedResourceBuilding(BuildingActor, BuildingActor->ResourceType, FinalResourceCount, false, WeakSpot);
		auto Inventory = PlayerController->GetInventory();
		auto Resource = UFortKismetLibrary::K2_GetResourceItemDefinition(BuildingActor->ResourceType);
		auto MaxStack = 500; // kismet helper is returning 999 so yh
		auto Item = Inventory->GetItemEntryByDef(Resource);

			if (Item)
			{
				Item->Count += FinalResourceCount;
				if (Item->Count > MaxStack)
				{
					FortKismetLibrary::SpawnPickup(Pawn->K2_GetActorLocation(), FortKismetLibrary::ConstructItemEntry(Resource, Item->Count - MaxStack, 0), EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset, Pawn, -1, true, true, true);
					Item->Count = MaxStack;
				}

				Inventory->ReplaceEntry(*Item);
			}
			else
			{
				if (FinalResourceCount > MaxStack)
				{
					FortKismetLibrary::SpawnPickup(Pawn->K2_GetActorLocation(), FortKismetLibrary::ConstructItemEntry(Resource, FinalResourceCount - MaxStack, 0), EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset,Pawn, -1, true, true, true);
					FinalResourceCount = MaxStack;
				}

				Inventory->AddItem(Resource, FinalResourceCount, 0);
			}

	} else
	{
		return false;
	}
	if (WeakSpot)
	{
		PlayerController->BroadcastRemoteClientInfo->RemoteWeakspotData.HitCount++;
	}
	else
		PlayerController->BroadcastRemoteClientInfo->RemoteWeakspotData.HitCount = 0;

    return true;
}
void BuildingSMActor::Init() {
    UHook* Hook = new UHook();

    Hook->Address = 0x1A2;
    Hook->Class = ABuildingSMActor::StaticClass();
    Hook->Detour = AttemptSpawnResources;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);
    
    delete Hook;
}