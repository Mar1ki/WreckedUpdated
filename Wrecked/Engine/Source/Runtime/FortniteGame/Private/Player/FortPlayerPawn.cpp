#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/Player/FortPlayerPawn.h"

#include <SDK/SDK/BP_PhoebePlayerController_classes.hpp>
#include "Engine/Source/Runtime/FortniteGame/Public/Kismet/FortKismetLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"

void FortPlayerPawn::ServerHandlePickupInfo(AFortPlayerPawn* Pawn, AFortPickup* Pickup, FFortPickupRequestInfo& Params)
{
    auto FlyTime = Params.FlyTime / Pawn->PickupSpeedMultiplier;
    Pickup->SetPickupTarget(Pawn, FlyTime, Params.Direction);
}

void FortPlayerPawn::ServerSendZiplineState(UObject* Context, FFrame& Params) {
    FZiplinePawnState State;

    Params.StepCompiledIn(&State);
    Params.IncrementCode();

    auto Pawn = (AFortPlayerPawn*)Context;
    if (!Pawn) return;

    auto Zipline = Pawn->GetActiveZipline();

    Pawn->ZiplineState = State;
    Pawn->HandleZiplineStateChanged();

    if (State.bIsZiplining && Zipline)
    {
        if (auto Ascender = Cast<AFortAscenderZipline>(Zipline))
        {
            Ascender->BP_HandlePlayerStartedUsingHandle(Pawn);
            
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

void FortPlayerPawn::TryAutoPickup(AFortPlayerPawn* Pawn, AFortPickupAthena* Pickup)
{
    if (!Pickup || !Pawn) return;
    
    auto ItemDefinition = (UFortItemDefinition*)Pickup->PrimaryPickupItemEntry.ItemDefinition;
    auto Controller  = (AFortPlayerControllerAthena*)Pawn->Controller;
    auto Inventory =  (AFortInventory*)Controller->WorldInventory.ObjectPointer;

    if (Controller->IsA(ABP_PhoebePlayerController_C::StaticClass()))
    {
        // actual logic for phoebe later
        return;
    }

    FFortPickupRequestInfo PickupInfo;
    PickupInfo.bIsAutoPickup = true;
    PickupInfo.bIsVisualOnlyPickup = false;
    PickupInfo.bPlayPickupSound = true;
    PickupInfo.bTrySwapWithWeapon = false;
    PickupInfo.bUseRequestedSwap = false;
    PickupInfo.FlyTime = Pickup->PickupLocationData.FlyTime;
    PickupInfo.Direction = Pickup->PickupLocationData.StartDirection;
    
    if (!Pawn->IsA(AFortPlayerPawnAthena::StaticClass())) return;
    
    if (Pickup->PawnWhoDroppedPickup != Pawn)
    {
        // here we check if the item is a consumable and we have it in the inv already so it will just stack
        if (ItemDefinition->ItemType == EFortItemType::Consumable)
        {
            auto MaxStack = FortKismetLibrary::GetMaxStackSize(ItemDefinition);
            auto ItemToStack = Inventory->GetItemEntryByDef(ItemDefinition);
            if (ItemToStack && ItemToStack->Count < MaxStack)
            {
                Pawn->ServerHandlePickupInfo(Pickup, PickupInfo);
            }
        }
        if (Pickup->PrimaryPickupItemEntry.ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass()))
        {
            if (ItemDefinition->ItemType == EFortItemType::WorldResource)
            {
                auto MaxStack = FortKismetLibrary::GetMaxStackSize(ItemDefinition);
                auto ItemToStack = Inventory->GetItemEntryByDef(ItemDefinition);
                if (ItemToStack && ItemToStack->Count < MaxStack)
                {
                    Pawn->ServerHandlePickupInfo(Pickup, PickupInfo);
                }
                else if (!ItemToStack || ItemToStack->Count < MaxStack)
                {
                    Pawn->ServerHandlePickupInfo(Pickup, PickupInfo);
                }
            }
        }
        if (Pickup->PrimaryPickupItemEntry.ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass())) 
        {
            Pawn->ServerHandlePickupInfo(Pickup, PickupInfo);
        }
    }
}

void FortPlayerPawn::ServerHoistDBNOPlayer(AFortPlayerPawn* Pawn, AFortPlayerPawn *InDBNOHoistee)
{
    ServerHoistDBNOPlayerOG(Pawn, InDBNOHoistee);
    if (!InDBNOHoistee)
        return;
    
    auto HoisterController = Pawn->Controller;
    if (!HoisterController)
        return;

    auto HoisterPlayerController = (AFortPlayerControllerAthena*)HoisterController;
    if (!HoisterPlayerController)
        return;

    if (!CanHoistFromDBNO(InDBNOHoistee, HoisterPlayerController))
        return;
    
    FFortDBNOCarryHoisterData PreviousHoisterData;
    PreviousHoisterData.DBNOHoister = nullptr;
    PreviousHoisterData.DBNOCarryEvent = EFortDBNOCarryEvent::PickedUp;
    
    InDBNOHoistee->bIsBeingDBNOCarried = true;
    
    InDBNOHoistee->DBNOHoisterData.DBNOHoister = Pawn;
    InDBNOHoistee->DBNOHoisterData.DBNOCarryEvent = EFortDBNOCarryEvent::PickedUp;
    Pawn->DBNOHoistee = InDBNOHoistee;
    
    InDBNOHoistee->OnRep_DBNOHoisterData(PreviousHoisterData);
    Pawn->OnRep_DBNOHoistee();
    
    InDBNOHoistee->ForceNetUpdate();
    Pawn->ForceNetUpdate();
}
void FortPlayerPawn::ServerThrowDBNOPlayer(UObject* Object, FFrame* Frame)
{
    ServerThrowDBNOPlayerOG(Object, Frame);
    
    auto Pawn = (AFortPlayerPawn*)Object;
    AFortPlayerPawn* CurrentHoistee = Pawn->DBNOHoistee;
    if (!CurrentHoistee)
        return;
    
    FFortDBNOCarryHoisterData PreviousHoisterData = CurrentHoistee->DBNOHoisterData;
    CurrentHoistee->bIsBeingDBNOCarried = false;
    CurrentHoistee->DBNOHoisterData.DBNOHoister = nullptr;
    CurrentHoistee->DBNOHoisterData.DBNOCarryEvent = EFortDBNOCarryEvent::Thrown;

    Pawn->DBNOHoistee = nullptr;
    Pawn->bIsDBNOCarrying = false;
    
    CurrentHoistee->OnRep_DBNOHoisterData(PreviousHoisterData);
    Pawn->OnRep_DBNOHoistee();

    CurrentHoistee->ForceNetUpdate();
    Pawn->ForceNetUpdate();
}

// 	void ServerDropCarriedPlayer(); need to impl that too :sob
// thats startdbno i think
void FortPlayerPawn::test(__int64* a1, __int64* a2)
{
    auto Pawn = (AFortPlayerPawn*)a1;
    Pawn->bIsDBNO = true;
    Pawn->OnRep_IsDBNO();   
    std::cout << "STARTDBNO i think" << std::endl;
    testOG(a1, a2);
}
void FortPlayerPawn::Init() {

    UHook* Hook = new UHook();

    Hook->Address = 0x24B;
    Hook->Class = AFortPlayerPawn::StaticClass();
    Hook->Detour = ServerHandlePickupInfo;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);

    Hook->Path = "/Script/FortniteGame.FortPlayerPawn.ServerSendZiplineState";
    Hook->Detour = ServerSendZiplineState;
    UKismetHookingLibrary::Hook(Hook, Exec);
    
    Hook->Address = ImageBase + 0xA149FB8;
    Hook->Detour = TryAutoPickup;
    UKismetHookingLibrary::Hook(Hook, Address);

    // broken rn
    Hook->Class = AFortPlayerPawn::StaticClass();
    Hook->Address = 0x11C0 / 8;
    Hook->Detour = ServerHoistDBNOPlayer;
    Hook->Original = (void**)&ServerHoistDBNOPlayerOG;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);

    Hook->Path = "/Script/FortniteGame.FortPlayerPawn.ServerThrowCarriedPlayer";
    Hook->Detour = ServerThrowDBNOPlayer;
    Hook->Original = (void**)&ServerThrowDBNOPlayerOG;
    UKismetHookingLibrary::Hook(Hook, Exec);
    
    Hook->Address = 3224 / 8;
    Hook->Class = AFortPlayerPawn::StaticClass();
    Hook->Detour = test;
    Hook->Original = (void**)&testOG;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);
    
    delete Hook;
}