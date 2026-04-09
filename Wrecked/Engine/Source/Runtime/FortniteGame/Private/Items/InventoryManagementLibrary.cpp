#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Items/InventoryManagementLibrary.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

bool InventoryManagementLibrary::RemoveItem(UObject* Object, FFrame& Params, bool* Return) {
    TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner;
    int32 Count;
    bool bForceRemoval;
    bool bForcePersistWhenEmpty;

    Params.StepCompiledIn(&InventoryOwner);
    FGuid& ItemGuid = Params.StepCompiledInRef<FGuid>();
    Params.StepCompiledIn(&Count);
    Params.StepCompiledIn(&bForceRemoval);
    Params.StepCompiledIn(&bForcePersistWhenEmpty);
    Params.IncrementCode();
	
    const AFortPawn* Pawn = ((const AFortPawn * (*)(void*))(*(void***)InventoryOwner.InterfacePointer)[9])(InventoryOwner.InterfacePointer);
    auto PlayerController = (AFortPlayerControllerAthena*)Pawn->Controller;
    auto Inventory = (AFortInventory*)PlayerController->WorldInventory.ObjectPointer;
    // um we check here if these exist bc i had it one time crash bc pawn was nullptr
    if (!Pawn || !Inventory || !PlayerController)
    {
        UE_LOG(LogInventoryManagmentLibrary, Warning, "PlayerContext is nullptr, returning false");
        *Return = false;
        return false;
    }
    
    auto ItemEntry = Inventory->GetItemEntryByGuid(ItemGuid);
    if (!ItemEntry)
    {
        UE_LOG(LogInventoryManagmentLibrary, Warning, "ItemEntry is nullptr, returning false");
        *Return = false;
        return false;
    }
    
    if (bForceRemoval)
    {
        Inventory->Remove(ItemGuid);
        *Return = true;
        return true;
    }

    // here is the start call when u start using the medkit or anything else using that tick system so for example medkit is calling with bForcePersistWhenEmpty and count 1 so we just change the count 
    if (bForcePersistWhenEmpty && Count > 0)
    {
        ItemEntry->Count -= Count;
        Inventory->Update(ItemEntry);
		
        auto* ItemInstance = Inventory->GetWorldItemByGuid(ItemGuid);
        if (ItemInstance)
        {
            (*ItemInstance)->ItemEntry.Count -= Count;
        }
        *Return = true;
        return true;
    }
    UE_LOG(LogInventoryManagmentLibrary, Verbose, "InventoryManagementLibrary::RemoveItem Called");
    // umm not proper but idc
    *Return = false;
    return false;
}

void InventoryManagementLibrary::RemoveItems(UObject* Object, FFrame& Params)
{
    TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner;
    Params.StepCompiledIn(&InventoryOwner);
    const TArray< FItemGuidAndCount> Items = Params.StepCompiledInRef<TArray< FItemGuidAndCount>>();
    Params.IncrementCode();
    
    const AFortPawn* Pawn = ((const AFortPawn * (*)(void*))(*(void***)InventoryOwner.InterfacePointer)[9])(InventoryOwner.InterfacePointer);
    auto PlayerController = (AFortPlayerControllerAthena*)Pawn->Controller;
    auto Inventory = (AFortInventory*)PlayerController->WorldInventory.ObjectPointer;
    
    if (!Pawn || !Inventory || !PlayerController)
    {
        UE_LOG(LogInventoryManagmentLibrary, Warning, "PlayerContext is nullptr, returning false");
        return;
    }
    // im not gonna add impl here until we make the inv proper
    std::cout << "InventoryManagementLibrary.RemoveItems Called, add impl pls" << std::endl;
}


void InventoryManagementLibrary::Init() {

    UHook* Hook = new UHook();
    
    Hook->Path = "/Script/FortniteGame.InventoryManagementLibrary.RemoveItem";
    Hook->Detour = RemoveItem;
    UKismetHookingLibrary::Hook(Hook, Exec);

  /*  Hook->Path = "/Script/FortniteGame.InventoryManagementLibrary.RemoveItems";
    Hook->Detour = RemoveItems;
    UKismetHookingLibrary::Hook(Hook, Exec);*/
}