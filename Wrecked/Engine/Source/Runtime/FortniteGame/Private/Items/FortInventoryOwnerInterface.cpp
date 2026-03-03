#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Items/FortInventoryOwnerInterface.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"


bool FortInventoryOwnerInterface::RemoveInventoryItem(IFortInventoryOwnerInterface* Interface, FGuid* ItemGuid, int Count)
{
    // this shit calls const getforpawn
    const AFortPawn* Pawn = ((const AFortPawn* (*)(IFortInventoryOwnerInterface*))(*(void***)Interface)[9])(Interface);
    if (!Pawn) return false;
    auto PlayerController = (AFortPlayerControllerAthena*)Pawn->Controller;
    auto Inventory = (AFortInventory*)PlayerController->WorldInventory.ObjectPointer;

    UE_LOG(LogFortInventoryOwnerInterface, Verbose, "FortInventoryOwnerInterface::RemoveInventoryItem Called");

    if (!PlayerController || !Inventory)
    {
        UE_LOG(LogFortInventoryOwnerInterface, Warning, "PlayerContext is nullptr, returning false");
        return false;
    }
    
    if (!UKismetGuidLibrary::IsValid_Guid(*ItemGuid))
    {
        UE_LOG(LogFortInventoryOwnerInterface, Warning, "ItemGuid is invalid, returning false");
        return false;
    }

    if (PlayerController->bInfiniteAmmo || PlayerController->bBuildFree)
    {
        return false;
    }
    
    auto ItemEntry = Inventory->GetItemEntryByGuid(*ItemGuid);
    if (!ItemEntry)
    {
        UE_LOG(LogFortInventoryOwnerInterface, Warning, "ItemEntry is invalid, returning false");
        return false;
    }
    ItemEntry->Count -= Count;
    auto Success =  Inventory->ReplaceEntry(*ItemEntry);
    if (!Success)
    {
        UE_LOG(LogFortInventoryOwnerInterface, Warning, "Failed to remove, item duplicated");
        return false;
    }
    return true;
}

bool __fastcall FortInventoryOwnerInterface::CreateInventory(IFortInventoryOwnerInterface* Interface) {
    const AFortPawn* Pawn = ((const AFortPawn * (*)(IFortInventoryOwnerInterface*))(*(void***)Interface)[9])(Interface);
    if (!Pawn) return false;
    auto PlayerController = (AFortPlayerControllerAthena*)Pawn->Controller;
    if (!PlayerController) return false;
    if (!PlayerController->WorldInventory.GetObjectRef()) {
        FActorSpawnParameters SpawnParams{};
        SpawnParams.NameMode = Required_Fatal;
        SpawnParams.ObjectFlags = EObjectFlags::Transactional;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.bNoFail = true;

        AFortInventory* Object = ((AFortInventory * (*)(UWorld*, UClass*, FTransform const*, FActorSpawnParameters*))(ImageBase + 0x1A8087C))(GetWorld(), PlayerController->WorldInventoryClass, new FTransform(FVector(), {}), &SpawnParams);
        Object->SetOwner(PlayerController);
        PlayerController->WorldInventory.InterfacePointer = Object->GetInterfaceAddress<IFortInventoryInterface>();
        PlayerController->WorldInventory.ObjectPointer = Object;
        PlayerController->bHasInitializedWorldInventory = true;
        return true;
    }
    return true;
}

void __fastcall FortInventoryOwnerInterface::InitializeInventory(IFortInventoryOwnerInterface* Interface, const TArray<FItemAndCount> ItemList) {
    const AFortPawn* Pawn = ((const AFortPawn * (*)(IFortInventoryOwnerInterface*))(*(void***)Interface)[9])(Interface);
    if (!Pawn) return;
    auto PlayerController = (AFortPlayerControllerAthena*)Pawn->Controller;

    if (PlayerController) {
        auto Inventory = PlayerController->GetInventory();
        if (Inventory) {
            for (auto Item : ItemList) {
                Inventory->AddItem((UFortItemDefinition*)Item.Item, Item.Count, 0, 1);
            }
        }
    }
}

AFortQuickBars* FortInventoryOwnerInterface::GetQuickBars(IFortInventoryOwnerInterface* Interface)
{
    const AFortPawn* Pawn = ((const AFortPawn * (*)(IFortInventoryOwnerInterface*))(*(void***)Interface)[9])(Interface);
    auto Controller = (AFortPlayerControllerAthena*)Pawn->Controller;

   return Controller->ClientQuickBars;
}

// we dont give a fuck about the item input so its prob unproper rn but works
bool FortInventoryOwnerInterface::HasInfiniteAmmo(IFortInventoryOwnerInterface* Interface, const UFortItemDefinition* Item)
{
    const AFortPawn* Pawn = ((const AFortPawn * (*)(IFortInventoryOwnerInterface*))(*(void***)Interface)[9])(Interface);
    auto Controller = (AFortPlayerControllerAthena*)Pawn->Controller;

    return Controller->bInfiniteAmmo;
}

bool FortInventoryOwnerInterface::HasInfiniteConsumables(IFortInventoryOwnerInterface* Interface)
{
    const AFortPawn* Pawn = ((const AFortPawn * (*)(IFortInventoryOwnerInterface*))(*(void***)Interface)[9])(Interface);
    auto Controller = (AFortPlayerControllerAthena*)Pawn->Controller;

    return Controller->bInfiniteConsumables;
}

void FortInventoryOwnerInterface::Init() {

    UHook* Hook = new UHook();

    Hook->Class = AFortPlayerControllerAthena::StaticClass();
    Hook->Interface = IFortInventoryOwnerInterface::StaticClass();
    Hook->Address = 440 / 8;
    Hook->Detour = RemoveInventoryItem;
    UKismetHookingLibrary::Hook(Hook, Interface);

    Hook->Class = AFortPlayerControllerAthena::StaticClass();
    Hook->Interface = IFortInventoryOwnerInterface::StaticClass();
    Hook->Address = 112 / 8;
    Hook->Detour = GetQuickBars;
    UKismetHookingLibrary::Hook(Hook, Interface);
    
    Hook->Class = AFortPlayerControllerAthena::StaticClass();
    Hook->Interface = IFortInventoryOwnerInterface::StaticClass();
    Hook->Address = 128 / 8;
    Hook->Detour = HasInfiniteAmmo;
    UKismetHookingLibrary::Hook(Hook, Interface);

    Hook->Class = AFortPlayerControllerAthena::StaticClass();
    Hook->Interface = IFortInventoryOwnerInterface::StaticClass();
    Hook->Address = 136 / 8;
    Hook->Detour = HasInfiniteConsumables;
    UKismetHookingLibrary::Hook(Hook, Interface);

 /*   Hook->Class = AFortPlayerControllerAthena::StaticClass();
    Hook->Interface = IFortInventoryOwnerInterface::StaticClass();
    Hook->Address = 0x18 / 8;
    Hook->Detour = CreateInventory;
    UKismetHookingLibrary::Hook(Hook, Interface);

    Hook->Class = AFortPlayerControllerAthena::StaticClass();
    Hook->Interface = IFortInventoryOwnerInterface::StaticClass();
    Hook->Address = 0x20 / 8;
    Hook->Detour = InitializeInventory;
    UKismetHookingLibrary::Hook(Hook, Interface); */
    delete Hook;
}