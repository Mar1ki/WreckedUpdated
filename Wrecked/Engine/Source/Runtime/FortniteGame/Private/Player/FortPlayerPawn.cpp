#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/Player/FortPlayerPawn.h"

#include "Engine/Source/Runtime/FortniteGame/Public/Kismet/FortKismetLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"

void FortPlayerPawn::ServerHandlePickupInfo(AFortPlayerPawn* Pawn, AFortPickup* Pickup, FFortPickupRequestInfo& Params)
{
    if (!Pickup || Pickup->bPickedUp)
        return;

    auto PlayerController = (AFortPlayerControllerAthena*)Pawn->GetController();

    if (!PlayerController || !Pickup->PrimaryPickupItemEntry.ItemDefinition)
        return;

    Pawn->IncomingPickups.Add(Pickup);

    Pickup->PickupLocationData.bPlayPickupSound = Params.bPlayPickupSound;
    Pickup->PickupLocationData.FlyTime = 0.40f;
    Pickup->PickupLocationData.ItemOwner = Pawn;
    Pickup->PickupLocationData.PickupGuid = Pickup->PrimaryPickupItemEntry.ItemGuid;
    Pickup->PickupLocationData.PickupTarget = Pawn;
    Pickup->PickupLocationData.StartDirection = (FVector_NetQuantizeNormal)Params.Direction;
    Pickup->OnRep_PickupLocationData();

    Pickup->bPickedUp = true;
    Pickup->OnRep_bPickedUp();
    return ServerHandlePickupInfoOG(Pawn, Pickup, Params);
}

void FortPlayerPawn::ServerSendZiplineState(UObject* Context, FFrame& Params) {
    FZiplinePawnState State;

    Params.StepCompiledIn(&State);
    Params.IncrementCode();

    auto Pawn = (AFortPlayerPawn*)Context;
    if (!Pawn) return;

    auto Zipline = Pawn->GetActiveZipline();

    Pawn->ZiplineState = State;

    static void (*HandleZiplineStateChanged)(AFortPlayerPawn*) = decltype(HandleZiplineStateChanged)(ImageBase + 0xA12F5D8);
    HandleZiplineStateChanged(Pawn);

    if (State.bIsZiplining && Zipline)
    {
        if (auto Ascender = Cast<AFortAscenderZipline>(Zipline))
        {
            Ascender->BP_HandlePlayerStoppedUsingHandle(Pawn); // same as below its prob stripped
            
            Ascender->PawnUsingHandle = Pawn;
            Ascender->OnRep_PawnUsingHandle();
        }
    }
    
    else if (!State.bIsZiplining && Zipline)
    {
        if (auto Ascender = Cast<AFortAscenderZipline>(Zipline))
        {
            Ascender->BP_HandlePlayerStoppedUsingHandle(Pawn); // i bet its stripped
            
            TWeakObjectPtr<AFortPlayerPawn> EmptyPtr{};
            EmptyPtr.ObjectIndex = -1;
            Ascender->PawnUsingHandle = EmptyPtr;
            Ascender->PreviousPawnUsingHandle = Pawn;
            Ascender->OnRep_PawnUsingHandle();
        }
    }
}

UFortWeaponModSetData* AFortPickup::GetWeaponModsetData() {
    for (auto State : this->PrimaryPickupItemEntry.StateValuesConstObject) {
        if (State.StateType.TagName == UKismetStringLibrary::Conv_StringToName(L"ItemStateValue.ConstObject.ModSet")) {
            return (UFortWeaponModSetData*)State.ObjectData.Get();
        }
    }
    return nullptr;
}

__int64* FortPlayerPawn::OnAboutToEnterBackpack(AFortPickup* Pickup)
{
    auto Pawn = Cast<AFortPlayerPawn>(Pickup->PickupLocationData.PickupTarget.Get());
    if (!Pawn) return OnAboutToEnterBackpackOG(Pickup);
    auto PC = Cast<AFortPlayerControllerAthena>(Pawn->GetController());
    if (!PC) return OnAboutToEnterBackpackOG(Pickup);

    AFortAthenaAIBotController* AI = Cast<AFortAthenaAIBotController>(PC);
    AFortInventory* Inv = Cast<AFortInventory>(PC->WorldInventory.GetObjectRef());
    if (!Inv) return OnAboutToEnterBackpackOG(Pickup);

    auto MyPawn = AI ? Cast<AFortPlayerPawn>(AI->PlayerBotPawn) : PC->MyFortPawn;
    if (!MyPawn) return OnAboutToEnterBackpackOG(Pickup);

    auto& PickupEntry = Pickup->PrimaryPickupItemEntry;
    auto Def = (UFortWorldItemDefinition*)PickupEntry.ItemDefinition;
    auto MaxStack = FortKismetLibrary::GetMaxStackSize(Def);
    auto& Items = Inv->Inventory.ItemInstances;

    int LoadedAmmo = PickupEntry.LoadedAmmo;
    int PhantomAmmo = PickupEntry.PhantomReserveAmmo;
    int ItemCount = 0;
    for (auto* I : Items)
        if (GetQuickbar(I->ItemEntry.ItemDefinition) == EFortQuickBars::Primary &&
            ((UFortItemDefinition*)I->ItemEntry.ItemDefinition)->bInventorySizeLimited)
            ItemCount++;

    bool bShouldSwap = (ItemCount - 6) >= 5;
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
                    auto NewItem = Inv->AddItem(Def, Overflow, LoadedAmmo, PickupEntry.Level, Pickup->GetWeaponModsetData());
                    if (NewItem && PhantomAmmo > 0) NewItem->PhantomReserveAmmo = PhantomAmmo;
                }
                else
                    FortKismetLibrary::SpawnPickup(PC->GetViewTarget()->K2_GetActorLocation(), PickupEntry,
                        EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, MyPawn, Overflow, true, false, true);
            }
            else Entry.Count = NewCount;
            Inv->ReplaceEntry(Entry);
        }
        else {
            if (PickupEntry.Count > MaxStack) {
                int Overflow = PickupEntry.Count - MaxStack;
                if (ItemCount == 5 && GetQuickbar(Def) == EFortQuickBars::Primary) {
                    auto CW = MyPawn->CurrentWeapon;
                    if (CW && GetQuickbar(CW->GetWeaponData()) == EFortQuickBars::Primary) {
                        FGuid CWGuid = CW->ItemEntryGuid;
                        for (auto* I : Items)
                            if (I->ItemEntry.ItemGuid == CWGuid) {
                                auto& CWEntry = I->ItemEntry;
                                FortKismetLibrary::SpawnPickup(MyPawn->K2_GetActorLocation() + MyPawn->GetActorForwardVector() * 70.f + FVector(0, 0, 50),
                                    CWEntry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, MyPawn, CWEntry.Count, true, false, true);
                                Inv->Remove(CWGuid);
                                break;
                            }
                    }
                    else {
                        FortKismetLibrary::SpawnPickup(PC->GetViewTarget()->K2_GetActorLocation(), PickupEntry,
                            EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, MyPawn, MaxStack, true, false, true);
                        return OnAboutToEnterBackpackOG(Pickup);
                    }
                    auto NewItem = Inv->AddItem(Def, MaxStack, LoadedAmmo, PickupEntry.Level, Pickup->GetWeaponModsetData());
                    if (NewItem && PhantomAmmo > 0) NewItem->PhantomReserveAmmo = PhantomAmmo;
                }
                else {
                    auto NewItem = Inv->AddItem(Def, MaxStack, LoadedAmmo, PickupEntry.Level, Pickup->GetWeaponModsetData());
                    if (NewItem && PhantomAmmo > 0) NewItem->PhantomReserveAmmo = PhantomAmmo;
                }

                if (Def->bAllowMultipleStacks && ItemCount < 5) {
                    auto NewItem = Inv->AddItem(Def, Overflow, LoadedAmmo, PickupEntry.Level, Pickup->GetWeaponModsetData());
                    if (NewItem && PhantomAmmo > 0) NewItem->PhantomReserveAmmo = PhantomAmmo;
                }
                else
                    FortKismetLibrary::SpawnPickup(PC->GetViewTarget()->K2_GetActorLocation(), PickupEntry,
                        EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, MyPawn, Overflow, true, false, true);
            }
            else {
                if (bShouldSwap && GetQuickbar(Def) == EFortQuickBars::Primary) {
                    auto CW = MyPawn->CurrentWeapon;
                    if (CW && GetQuickbar(CW->GetWeaponData()) == EFortQuickBars::Primary) {
                        FGuid CWGuid = CW->ItemEntryGuid;
                        for (auto* I : Items)
                            if (I->ItemEntry.ItemGuid == CWGuid) {
                                auto& CWEntry = I->ItemEntry;
                                FortKismetLibrary::SpawnPickup(MyPawn->K2_GetActorLocation() + MyPawn->GetActorForwardVector() * 70.f + FVector(0, 0, 50),
                                    CWEntry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, MyPawn, CWEntry.Count, true, false, true);
                                Inv->Remove(CWGuid);
                                break;
                            }
                        auto NewItem = Inv->AddItem(Def, PickupEntry.Count, LoadedAmmo, PickupEntry.Level, Pickup->GetWeaponModsetData());
                        if (NewItem && PhantomAmmo > 0) NewItem->PhantomReserveAmmo = PhantomAmmo;
                    }
                    else FortKismetLibrary::SpawnPickup(PC->GetViewTarget()->K2_GetActorLocation(), PickupEntry,
                        EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, MyPawn, MaxStack, true, false, true);
                }
                else {
                    auto NewItem = Inv->AddItem(Def, PickupEntry.Count, LoadedAmmo, PickupEntry.Level, Pickup->GetWeaponModsetData());
                    if (NewItem && PhantomAmmo > 0) NewItem->PhantomReserveAmmo = PhantomAmmo;
                }
            }
        }
    }
    else {
        if (ItemCount == 5 && GetQuickbar(Def) == EFortQuickBars::Primary) {
            auto CW = MyPawn->CurrentWeapon;
            if (CW && GetQuickbar(CW->GetWeaponData()) == EFortQuickBars::Primary) {
                FGuid CWGuid = CW->ItemEntryGuid;
                for (auto* I : Items)
                    if (I->ItemEntry.ItemGuid == CWGuid) {
                        auto& CWEntry = I->ItemEntry;
                        FortKismetLibrary::SpawnPickup(MyPawn->K2_GetActorLocation() + MyPawn->GetActorForwardVector() * 70.f + FVector(0, 0, 50),
                            CWEntry, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, MyPawn, CWEntry.Count, true, false, true);
                        Inv->Remove(CWGuid);
                        break;
                    }
                auto NewItem = Inv->AddItem(Def, PickupEntry.Count, LoadedAmmo, PickupEntry.Level, Pickup->GetWeaponModsetData());
                if (NewItem && PhantomAmmo > 0) NewItem->PhantomReserveAmmo = PhantomAmmo;
            }
            else FortKismetLibrary::SpawnPickup(PC->GetViewTarget()->K2_GetActorLocation(), PickupEntry,
                EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::Unset, MyPawn, MaxStack, true, false, true);
        }
        else {
            auto NewItem = Inv->AddItem(Def, PickupEntry.Count, LoadedAmmo, PickupEntry.Level, Pickup->GetWeaponModsetData());
            if (NewItem && PhantomAmmo > 0) NewItem->PhantomReserveAmmo = PhantomAmmo;
        }
    }

    return OnAboutToEnterBackpackOG(Pickup);
}

void FortPlayerPawn::TryAutoPickup(AFortPlayerPawn* Pawn, AFortPickupAthena* Pickup)
{
    if (!Pickup || !Pawn) return;
    
    auto ItemDefinition = (UFortItemDefinition*)Pickup->PrimaryPickupItemEntry.ItemDefinition;
    auto Controller  = (AFortPlayerControllerAthena*)Pawn->Controller;
    auto Inventory =  (AFortInventory*)Controller->WorldInventory.ObjectPointer;
    
    if (Pickup->PawnWhoDroppedPickup != Pawn)
    {
        // here we check if the item is a consumable and we have it in the inv already so it will just stack
        if (ItemDefinition->ItemType == EFortItemType::Consumable)
        {
            auto ItemToStack = Inventory->GetItemEntryByDef(ItemDefinition);
            if (ItemToStack)
            {
                Pawn->ServerHandlePickup(Pickup, Pickup->PickupLocationData.FlyTime, Pickup->PickupLocationData.StartDirection, Pickup->PickupLocationData.bPlayPickupSound);
            }
        }
        if (Pickup->PrimaryPickupItemEntry.ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()) || Pickup->PrimaryPickupItemEntry.ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass())) 
        {
            Pawn->ServerHandlePickup(Pickup, Pickup->PickupLocationData.FlyTime, Pickup->PickupLocationData.StartDirection, Pickup->PickupLocationData.bPlayPickupSound);
        }
    }
}
void FortPlayerPawn::Init() {

    UHook* Hook = new UHook();

    Hook->Address = 0x24B;
    Hook->Class = AFortPlayerPawn::StaticClass();
    Hook->Detour = ServerHandlePickupInfo;
    Hook->Original = (void**)&ServerHandlePickupInfoOG;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);

    Hook->Address = ImageBase + 0x2784A48;
    Hook->Detour = OnAboutToEnterBackpack;
    Hook->Original = (void**)&OnAboutToEnterBackpackOG;
    UKismetHookingLibrary::Hook(Hook, Address);

    Hook->Path = "/Script/FortniteGame.FortPlayerPawn.ServerSendZiplineState";
    Hook->Detour = ServerSendZiplineState;
    UKismetHookingLibrary::Hook(Hook, Exec);
    
    Hook->Address = ImageBase + 0xA149FB8;
    Hook->Detour = TryAutoPickup;
    UKismetHookingLibrary::Hook(Hook, Address); 
    
    delete Hook;
}