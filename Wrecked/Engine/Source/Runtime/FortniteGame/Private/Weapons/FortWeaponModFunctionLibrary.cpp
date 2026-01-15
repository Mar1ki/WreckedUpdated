#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Weapons/FortWeaponModFunctionLibrary.h"

#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Kismet/FortKismetLibrary.h"

UFortWeaponModSetData* FortWeaponModFunctionLibrary::GetRandomModsetForWeapon(UFortWeaponItemDefinition* Weapon) {
    auto static ModSetBucketDataTable = StaticLoadObject<UDataTable>("/WeaponMods/DataTables/ModSetBucketsBaseTable.ModSetBucketsBaseTable");

    if (!Weapon)
        return nullptr;

    if (!Weapon->bIsModifiableWeapon)
        return nullptr;

    FName ModTagName;

    for (auto Tag : Weapon->GameplayTags.GameplayTags) {
        if (Tag.TagName.ToString().contains("ModSetBucket")) {
            ModTagName = Tag.TagName;
        }
    }
    std::map<FName, FFortWeaponModSetRow*> OutMap;
    float TotalWeight = 0;
    for (TPair<FName, uint8_t*>& CurrentPair : ModSetBucketDataTable->RowMap)
    {
        if (((FFortWeaponModSetRow*)CurrentPair.Value())->BucketTag.TagName == ModTagName)
        {
            OutMap.insert_or_assign(CurrentPair.Key(), (FFortWeaponModSetRow*)CurrentPair.Value());
            TotalWeight += ((FFortWeaponModSetRow*)CurrentPair.Value())->RandomWeight;
        }
    }

    double accumulated = 0.0;
    float randomValue = WeightedRandom(TotalWeight);
    for (const auto& pair : OutMap) {
        accumulated += pair.second->RandomWeight;
        if (randomValue < accumulated) {
            return pair.second->Set;
        }
    }
    return nullptr;
}