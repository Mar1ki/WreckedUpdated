#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/Player/FortPlayerPawn.h"

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
            }
        }
        if (Pickup->PrimaryPickupItemEntry.ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass())) 
        {
            Pawn->ServerHandlePickupInfo(Pickup, PickupInfo);
        }
    }
}

void ServerHoistDBNOPlayer(UObject* Object, FFrame* Frame)
{
    AFortPlayerPawn* InDBNOHoistee;
    Frame->StepCompiledIn(&InDBNOHoistee);
    Frame->IncrementCode();

    auto Pawn = (AFortPlayerPawn*)Object;
    auto previous = Pawn->DBNOHoisterData;

    Pawn->DBNOHoistee = InDBNOHoistee;
    Pawn->DBNOHoisterData.DBNOCarryEvent = EFortDBNOCarryEvent::PickedUp;
    Pawn->DBNOHoisterData.DBNOHoister = Pawn;

    Pawn->OnRep_DBNOHoisterData(previous);
    Pawn->OnRep_DBNOHoistee();

    InDBNOHoistee->bIsBeingDBNOCarried = true;
}

void ServerThrowCarriedPlayer(UObject* Object, FFrame* Frame)
{

    auto Pawn = (AFortPlayerPawn*)Object;
    auto previous = Pawn->DBNOHoisterData;

    auto InDBNOHoistee = Pawn->GetDBNOHoistee();
    Pawn->DBNOHoistee = InDBNOHoistee;
    Pawn->DBNOHoisterData.DBNOHoister = Pawn;
    Pawn->DBNOHoisterData.DBNOCarryEvent = EFortDBNOCarryEvent::Thrown;

    Pawn->LocalThrowCarriedPlayer();

    InDBNOHoistee->bIsBeingDBNOCarried = false;

    Pawn->OnRep_DBNOHoisterData(previous);
    Pawn->OnRep_DBNOHoistee();
}
// thats startdbno i think
void FortPlayerPawn::test(__int64* a1, __int64* a2)
{
    std::cout << "helo" << std::endl;
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
   /* Hook->Path = "/Script/FortniteGame.FortPlayerPawn.ServerHoistDBNOPlayer";
    Hook->Detour = ServerHoistDBNOPlayer;
    UKismetHookingLibrary::Hook(Hook, Exec);

    Hook->Path = "/Script/FortniteGame.FortPlayerPawn.ServerThrowCarriedPlayer";
    Hook->Detour = ServerThrowCarriedPlayer;
    UKismetHookingLibrary::Hook(Hook, Exec);*/

    Hook->Address = 3224 / 8;
    Hook->Class = AFortPlayerPawn::StaticClass();
    Hook->Detour = test;
    Hook->Original = (void**)&testOG;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);
    
    delete Hook;
}