#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Items/FortPickup.h"

#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Kismet/FortKismetLibrary.h"


void FortPickup::GivePickupTo(AFortPickup* Pickup, IFortInventoryOwnerInterface* InventoryOwner, bool DestoryAfterPickup, double _XMM3_8)
{

    const auto Pawn = (AFortPlayerPawn*)InventoryOwner->GetPawn();
    auto PlayerController = (AFortPlayerControllerAthena*)Pawn->Controller;
    auto Inventory = PlayerController->GetInventory();

    auto& PickupEntry = Pickup->PrimaryPickupItemEntry;
    auto Def = (UFortWorldItemDefinition*)PickupEntry.ItemDefinition;
    auto MaxStack = FortKismetLibrary::GetMaxStackSize(Def);
    auto& Items = Inventory->Inventory.ItemInstances;

    int LoadedAmmo = PickupEntry.LoadedAmmo;
    int PhantomAmmo = PickupEntry.PhantomReserveAmmo;
    int ItemCount = 0;
    for (auto* I : Items)
        if (GetQuickbar(I->ItemEntry.ItemDefinition) == EFortQuickBars::Primary &&
            ((UFortItemDefinition*)I->ItemEntry.ItemDefinition)->bInventorySizeLimited)
            ItemCount++;
    int (*CountInventoryOverflow)(IFortInventoryOwnerInterface*, FFortItemEntry*, bool, bool) = decltype(CountInventoryOverflow)((*(void***)(InventoryOwner))[0x1C0/8]);
    auto Overflow = CountInventoryOverflow(InventoryOwner, &PickupEntry, true, true);
    bool bShouldSwap = ItemCount >= 5;
    UE_LOG(LogInventory, Log, "Attemptry To Pickup: %s ItemCount %i Overflow %i", Def->GetName().c_str(), ItemCount, Overflow);
    bool bIsStackable = Def->IsStackable();

    if (bIsStackable) {
        UFortWorldItem* ExistingItem = nullptr;
        for (auto* I : Items)
            if (I && I->ItemEntry.ItemDefinition == Def && I->ItemEntry.Count <= MaxStack)
            {
                ExistingItem = I; break;
            }

        if (ExistingItem) {
            auto& Entry = ExistingItem->ItemEntry;
            FFortItemEntryStateValue* State = nullptr;
            for (auto& Value : Entry.StateValues)
                if (Value.StateType == EFortItemEntryState::ShouldShowItemToast)
                {
                    State = &Value; break;
                }

            if (!State) {
                FFortItemEntryStateValue Value{};
                Value.StateType = EFortItemEntryState::ShouldShowItemToast;
                Value.IntValue = true;
                Entry.StateValues.Add(Value);
            }
            else State->IntValue = true;

            int NewCount = Entry.Count + PickupEntry.Count;
            if (NewCount > MaxStack) {
                Entry.Count = MaxStack;
                int Overflow = NewCount - MaxStack;
                if (Def->bAllowMultipleStacks && ItemCount < 5) {
                    auto NewItem = Inventory->AddItem(Def, Overflow, LoadedAmmo, PickupEntry.Level, Pickup->GetWeaponModsetData());
                    if (NewItem && PhantomAmmo > 0) NewItem->PhantomReserveAmmo = PhantomAmmo;
                }
                else
                    FortKismetLibrary::SpawnPickup(PlayerController->GetViewTarget()->K2_GetActorLocation(), PickupEntry,
                        EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn, Overflow, true, false, true);
            }
            else Entry.Count = NewCount;
            Inventory->ReplaceEntry(Entry);
        }
        else {
            if (PickupEntry.Count > MaxStack) {
                int Overflow = PickupEntry.Count - MaxStack;
                if (ItemCount == 5 && GetQuickbar(Def) == EFortQuickBars::Primary) {
                    auto CW = Pawn->CurrentWeapon;
                    if (CW && GetQuickbar(CW->GetWeaponData()) == EFortQuickBars::Primary) {
                        FGuid CWGuid = CW->ItemEntryGuid;
                        for (auto* I : Items)
                            if (I->ItemEntry.ItemGuid == CWGuid) {
                                auto& CWEntry = I->ItemEntry;
                                FortKismetLibrary::SpawnPickup(Pawn->K2_GetActorLocation() + Pawn->GetActorForwardVector() * 70.f + FVector(0, 0, 50),
                                    CWEntry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn, CWEntry.Count, true, false, true);
                                Inventory->Remove(CWGuid);
                                break;
                            }
                    }
                    else {
                        FortKismetLibrary::SpawnPickup(PlayerController->GetViewTarget()->K2_GetActorLocation(), PickupEntry,
                            EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn, MaxStack, true, false, true);
                         GivePickupToOG(Pickup, InventoryOwner, DestoryAfterPickup, _XMM3_8);
                    }
                    auto NewItem = Inventory->AddItem(Def, MaxStack, LoadedAmmo, PickupEntry.Level, Pickup->GetWeaponModsetData());
                    if (NewItem && PhantomAmmo > 0) NewItem->PhantomReserveAmmo = PhantomAmmo;
                }
                else {
                    auto NewItem = Inventory->AddItem(Def, MaxStack, LoadedAmmo, PickupEntry.Level, Pickup->GetWeaponModsetData());
                    if (NewItem && PhantomAmmo > 0) NewItem->PhantomReserveAmmo = PhantomAmmo;
                }

                if (Def->bAllowMultipleStacks && ItemCount < 5) {
                    auto NewItem = Inventory->AddItem(Def, Overflow, LoadedAmmo, PickupEntry.Level, Pickup->GetWeaponModsetData());
                    if (NewItem && PhantomAmmo > 0) NewItem->PhantomReserveAmmo = PhantomAmmo;
                }
                else
                    FortKismetLibrary::SpawnPickup(PlayerController->GetViewTarget()->K2_GetActorLocation(), PickupEntry,
                        EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn, Overflow, true, false, true);
            }
            else {

                if (bShouldSwap && GetQuickbar(Def) == EFortQuickBars::Primary ) {
                    UE_LOG(LogInventory, Log, "Check passed!");
                    auto CW = Pawn->CurrentWeapon;
                    if (CW && GetQuickbar(CW->GetWeaponData()) == EFortQuickBars::Primary) {
                        FGuid CWGuid = CW->ItemEntryGuid;
                        for (auto* I : Items)
                            if (I->ItemEntry.ItemGuid == CWGuid) {
                                auto& CWEntry = I->ItemEntry;
                                FortKismetLibrary::SpawnPickup(Pawn->K2_GetActorLocation() + Pawn->GetActorForwardVector() * 70.f + FVector(0, 0, 50),
                                    CWEntry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn, CWEntry.Count, true, false, true);
                                Inventory->Remove(CWGuid);
                                break;
                            }
                        auto NewItem = Inventory->AddItem(Def, PickupEntry.Count, LoadedAmmo, PickupEntry.Level, Pickup->GetWeaponModsetData());
                        if (NewItem && PhantomAmmo > 0) NewItem->PhantomReserveAmmo = PhantomAmmo;
                    }
                    else FortKismetLibrary::SpawnPickup(PlayerController->GetViewTarget()->K2_GetActorLocation(), PickupEntry,
                        EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn, MaxStack, true, false, true);
                }
                else {
                    auto NewItem = Inventory->AddItem(Def, PickupEntry.Count, LoadedAmmo, PickupEntry.Level, Pickup->GetWeaponModsetData());
                    if (NewItem && PhantomAmmo > 0) NewItem->PhantomReserveAmmo = PhantomAmmo;
                }
            }
        }
    }
    else {

        if (ItemCount == 5 && GetQuickbar(Def) == EFortQuickBars::Primary) {
            UE_LOG(LogInventory, Log, "Checkpassed!");
            auto CW = Pawn->CurrentWeapon;
            if (CW && GetQuickbar(CW->GetWeaponData()) == EFortQuickBars::Primary) {
                FGuid CWGuid = CW->ItemEntryGuid;
                for (auto* I : Items)
                    if (I->ItemEntry.ItemGuid == CWGuid) {
                        auto& CWEntry = I->ItemEntry;
                        FortKismetLibrary::SpawnPickup(Pawn->K2_GetActorLocation() + Pawn->GetActorForwardVector() * 70.f + FVector(0, 0, 50),
                            CWEntry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn, CWEntry.Count, true, false, true);
                        Inventory->Remove(CWGuid);
                        break;
                    }
                auto NewItem = Inventory->AddItem(Def, PickupEntry.Count, LoadedAmmo, PickupEntry.Level, Pickup->GetWeaponModsetData());
                if (NewItem && PhantomAmmo > 0) NewItem->PhantomReserveAmmo = PhantomAmmo;
            }
            else FortKismetLibrary::SpawnPickup(PlayerController->GetViewTarget()->K2_GetActorLocation(), PickupEntry,
                EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, Pawn, MaxStack, true, false, true);
        }
        else {
            auto NewItem = Inventory->AddItem(Def, PickupEntry.Count, LoadedAmmo, PickupEntry.Level, Pickup->GetWeaponModsetData());
            if (NewItem && PhantomAmmo > 0) NewItem->PhantomReserveAmmo = PhantomAmmo;
        }
    }




    GivePickupToOG(Pickup, InventoryOwner, DestoryAfterPickup, _XMM3_8);
}
void FortPickup::Init()
{
    UHook* Hook = new UHook();
    
    Hook->Class = AFortPickup::StaticClass();
    Hook->Address = 0x740 / 8;
    Hook->Detour = GivePickupTo;
    Hook->Original = (void**)&GivePickupToOG;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);

}