#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Items/FortPickup.h"

#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"


void FortPickup::GivePickupTo(AFortPickup* Pickup, IFortInventoryOwnerInterface* InventoryOwner, bool DestoryAfterPickup, double _XMM3_8)
{

    auto aaaaa = InventoryOwner->GetPawn();
    const AFortPawn* Pawn = ((const AFortPawn* (*)(IFortInventoryOwnerInterface*))(*(void***)InventoryOwner)[9])(InventoryOwner);
    auto PlayerController = (AFortPlayerControllerAthena*)Pawn->Controller;
    auto Inventory = PlayerController->GetInventory();
    
    std::cout << "givepickupto" << Pawn->Name.ToString().c_str()  << std::endl;
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