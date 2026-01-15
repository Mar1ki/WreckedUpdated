#include "pch.h"

#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Building/BuildingContainer.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Kismet/FortKismetLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"

bool BuildingContainer::SpawnLoot(ABuildingContainer* Container)
{
	for (auto Pair : GetGameMode()->RedirectAthenaLootTierGroups)
	{
		if (Pair.First == Container->SearchLootTierGroup)
		{
			Container->SearchLootTierGroup = Pair.Second;
			break;
		}
	}

	for (auto& Drop : FortKismetLibrary::PickLootDrops(Container->SearchLootTierGroup, GetGameState()->WorldLevel))
	{
		if (!Drop.ItemDefinition || Drop.Count <= 0)
			continue;
		FVector Loc = Container->K2_GetActorLocation() + (Container->GetActorForwardVector() * Container->LootSpawnLocation_Athena.X) + (Container->GetActorRightVector() * Container->LootSpawnLocation_Athena.Y) + (Container->GetActorUpVector() * Container->LootSpawnLocation_Athena.Z);
		FortKismetLibrary::SpawnPickup(
			Loc,
			Drop,
			EFortPickupSourceTypeFlag::Container,
			EFortPickupSpawnSource::Chest,
			nullptr,
			-1, true, true, true);
	//	std::cout << "LoadedAmmo: " << Drop.LoadedAmmo << " | PhantomAmmo: " << Drop.PhantomReserveAmmo << " | Count: " << Drop.Count << " | Item: " << Drop.ItemDefinition->Name.ToString().c_str() << std::endl;

		
	}

	Container->bAlreadySearched = true;
	Container->OnRep_bAlreadySearched();
	Container->SearchBounceData.SearchAnimationCount++;
	Container->BounceContainer();

	return true;
}
void BuildingContainer::OnDied_Event(UObject* Context, FFrame& Params) {
	AActor* DamagedActor;
	float Damage;
	APlayerController* InstigatedBy;
	AActor* DamageCauser;
	FVector HitLocation;
	UPrimitiveComponent* FHitComponent;
	FName BoneName;
	FVector Momentum;

	Params.StepCompiledIn(&DamagedActor);
	Params.StepCompiledIn(&Damage);
	Params.StepCompiledIn(&InstigatedBy);
	Params.StepCompiledIn(&DamageCauser);
	Params.StepCompiledIn(&HitLocation);
	Params.StepCompiledIn(&FHitComponent);
	Params.StepCompiledIn(&BoneName);
	Params.StepCompiledIn(&Momentum);
	Params.IncrementCode();
	std::cout << "IDK uhh\n";
	if (DamagedActor && DamageCauser) {
		auto static ModBoxClass = StaticLoadObject<UClass>("/VehicleMods/Gameplay/Actors/ModBox/ModBox_Parent.ModBox_Parent_C", UBlueprintGeneratedClass::StaticClass());
		if (DamagedActor->IsA(ModBoxClass)) {
			auto ModTag = *(FGameplayTag*)(__int64(DamagedActor) + 2464);
			UFortVehicleModConfig* ModConfig = nullptr;
			for (int i = 0; i < UObject::GObjects->Num(); i++)
			{
				auto Object = UObject::GObjects->GetByIndex(i);

				if (!Object)
					continue;

				if (Object->IsA(UFortVehicleModConfig::StaticClass()) && Object->IsDefaultObject())
				{
					auto ModConfig1 = (UFortVehicleModConfig*)Object;
					if (ModConfig1->ModTag == ModTag)
					{
						ModConfig = ModConfig1;
						break;
					}
				}
			}
			if (ModConfig != nullptr) {
				std::cout << ModConfig->GetName().c_str() << "\n";
			}

		}
	}

	OnDied_EventOG(Context, Params);
}
void BuildingContainer::OnDamageServer(ABuildingSMActor* BuildingActor, float Damage, FGameplayTagContainer& DamageTags, FVector& Momentum, FHitResult& HitInfo, AFortPlayerControllerAthena* InstigatedBy, AActor* DamageCauser, FGameplayEffectContextHandle& EffectContext)
{
	if (!BuildingActor || !InstigatedBy || !BuildingActor->IsA(ABuildingSMActor::StaticClass()) || BuildingActor->bPlayerPlaced || !DamageCauser || !DamageCauser->IsA(AFortWeapon::StaticClass()) || !((AFortWeapon*)DamageCauser)->WeaponData)
		return OnDamageServerOG(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

	auto ClassData = GetSparseData<FBuildingSMActorClassData>(BuildingActor->Class);

	if (!ClassData)
		return OnDamageServerOG(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

	if (ClassData->BuildingResourceAmountOverride.RowName.ToString() == "NAME_None")
		return OnDamageServerOG(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

	if (((AFortWeapon*)DamageCauser)->WeaponData->IsA(UFortWeaponMeleeItemDefinition::StaticClass()))
	{
		AFortPlayerControllerAthena* PlayerController = (AFortPlayerControllerAthena*)InstigatedBy;
		if (!PlayerController)
			return OnDamageServerOG(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

		static UCurveTable* CurveTable = StaticFindObject<UCurveTable>("/Game/Balance/DataTables/ResourceRates.ResourceRates");;

		if (!CurveTable)
			return OnDamageServerOG(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

		float Average = 1.f;
		EEvaluateCurveTableResult Result = {};

		UDataTableFunctionLibrary::EvaluateCurveTableRow(CurveTable, ClassData->BuildingResourceAmountOverride.RowName, 0.f, &Result, &Average, FString());
		float FinalResourceCount = round(Average / (BuildingActor->GetMaxHealth() / Damage));

		auto Inventory = (AFortInventory*)PlayerController->WorldInventory.ObjectPointer;
		if (FinalResourceCount > 0)
		{
			PlayerController->ClientReportDamagedResourceBuilding(BuildingActor, BuildingActor->ResourceType, FinalResourceCount, false, Damage == 100.f);
			auto Resource = UFortKismetLibrary::K2_GetResourceItemDefinition(BuildingActor->ResourceType);
			auto MaxStack = FortKismetLibrary::GetMaxStackSize(Resource);
			auto Item = Inventory->Inventory.ReplicatedEntries.Search([&](FFortItemEntry& entry) {
				return entry.ItemDefinition == Resource;
				});

			if (Item)
			{
				Item->Count += FinalResourceCount;
				if (Item->Count > MaxStack)
				{
					FortKismetLibrary::SpawnPickup(InstigatedBy->Pawn->K2_GetActorLocation(), FortKismetLibrary::ConstructItemEntry(Resource, Item->Count - MaxStack, 0), EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset, InstigatedBy->MyFortPawn, -1, true, true, true);
					Item->Count = MaxStack;
				}

				Inventory->ReplaceEntry(*Item);
			}
			else
			{
				if (FinalResourceCount > MaxStack)
				{
					FortKismetLibrary::SpawnPickup(InstigatedBy->Pawn->K2_GetActorLocation(), FortKismetLibrary::ConstructItemEntry(Resource, Item->Count - MaxStack, 0), EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset, InstigatedBy->MyFortPawn, -1, true, true, true);
					FinalResourceCount = MaxStack;
				}

				Inventory->AddItem(Resource, FinalResourceCount, 0);
			}

		}

		if (Damage == 100)
		{
			PlayerController->BroadcastRemoteClientInfo->RemoteWeakspotData.HitCount++;
		}
		else
			PlayerController->BroadcastRemoteClientInfo->RemoteWeakspotData.HitCount = 0;
	}

	return OnDamageServerOG(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
}


void BuildingContainer::Setup() {
	UHook* Hook = new UHook();

	Hook->Address = ImageBase + 0x934B6D8;
	Hook->Original = (void**)&SpawnLootOG;
	Hook->Detour = SpawnLoot;
	UKismetHookingLibrary::Hook(Hook, EHook::Address);
	
	Hook->Address = ImageBase + 0x93445F4;
	Hook->Original = (void**)&OnDamageServerOG;
	Hook->Detour = OnDamageServer;
	UKismetHookingLibrary::Hook(Hook, EHook::Address);

	auto Class = StaticLoadObject<UClass>("/VehicleMods/Gameplay/Actors/ModBox/ModBox_Parent.ModBox_Parent_C", UBlueprintGeneratedClass::StaticClass());
	auto Function = Class->GetFunction("ModBox_Parent_C", "OnDied_Event");
	if (Function) {
		auto idk = (void**)&OnDied_EventOG;
		*idk = Function->ExecFunction;
		Function->ExecFunction = reinterpret_cast<UFunction::FNativeFuncPtr>(OnDied_Event);

	}
	free(Hook);
}