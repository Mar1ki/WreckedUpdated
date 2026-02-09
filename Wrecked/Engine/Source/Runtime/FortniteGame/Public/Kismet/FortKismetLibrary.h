#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

static EFortQuickBars GetQuickbar(UItemDefinitionBase* ItemDefinition)
{
	if (!ItemDefinition)
		return EFortQuickBars::Max_None;
	if (!ItemDefinition->IsA(UFortWeaponMeleeItemDefinition::StaticClass()) &&
		!ItemDefinition->IsA(UFortBuildingItemDefinition::StaticClass()) &&
		!ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()) &&
		!ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass()) &&
		!ItemDefinition->IsA(UFortTrapItemDefinition::StaticClass()))
	{
		return EFortQuickBars::Primary;
	}

	return EFortQuickBars::Secondary;
}

static bool IsModSetEmpty(const UFortWeaponModSetData* ModSetData)
{
	if (!ModSetData)
		return true;
    
	return ModSetData->CommonRarityMods.Num() == 0 &&
		   ModSetData->UncommonRarityMods.Num() == 0 &&
		   ModSetData->RareRarityMods.Num() == 0 &&
		   ModSetData->EpicRarityMods.Num() == 0 &&
		   ModSetData->LengdaryRarityMods.Num() == 0;
}

namespace FortKismetLibrary
{
	FFortItemEntry ConstructItemEntry(UFortItemDefinition* ItemDefinition, int32 Count, int32 Level);
	int GetMaxStackSize(UFortItemDefinition* Item);
	TArray<FFortItemEntry> PickLootDrops(FName TierGroupName, int WorldLevel = 0);
	AFortPickupAthena* SpawnPickup(FVector Loc, FFortItemEntry Entry, EFortPickupSourceTypeFlag SourceTypeFlag, EFortPickupSpawnSource SpawnSource, class AFortPlayerPawn* Pawn, int OverrideCount, bool Toss, bool RandomRotation, bool bCombine);
	AFortPickupAthena* SpawnPickup(FVector Loc, UFortItemDefinition* Def, EFortPickupSourceTypeFlag SourceTypeFlag, EFortPickupSpawnSource SpawnSource, int Count = 1, int LoadedAmmo = 0, AFortPawn* Pawn = nullptr, TArray<FFortWeaponModSlot> WeaponSlots = {});
	static bool PickLootDropsHook(UObject*, FFrame&, bool*);
	static AFortPickup* K2_SpawnPickupInWorldWithClassHook(UObject*, FFrame& Stack, AFortPickup** Ret);
	static AFortPickup* K2_SpawnPickupInWorldHook(UObject*, FFrame&, AFortPickup**);
	static AFortPickup* SpawnItemVariantPickupInWorldHook(UObject*, FFrame&, AFortPickup**);
	static AFortPickup* K2_SpawnPickupInWorldWithClassAndItemEntry(UObject*, FFrame&, AFortPickup**);

	static void K2_RemoveItemFromPlayer(UObject* Object, FFrame& Params, int32* Ret);
	UFortWorldItem* GiveItemToInventoryOwner(UObject* Object, FFrame& Params);
	
	DefineOriginal(bool, OpenActorInternal, AActor* OpenableInterfaceActor , AFortPlayerControllerAthena* OptionalControllerInstigator,bool bRequestFastOpen);
	DefineOriginal(int32, K2_RemoveFortItemFromPlayer, UObject* Object, FFrame& Stack, int32* Ret);

	
	void Init();
}