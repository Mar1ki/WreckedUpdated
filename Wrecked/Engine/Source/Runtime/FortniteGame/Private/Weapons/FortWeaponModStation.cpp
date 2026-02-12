#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Weapons/FortWeaponModStation.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"


// alr its not proper bc it breakes after some time but we need to figure out why maybe another hook 
__int64 FortWeaponModStation::ServerStopInteractWithWorkbenchActor(UFortWeaponModStationComponent* Component, AFortWeaponModStationBase* ModStation)
{
    auto PlayerController = (AFortPlayerControllerAthena*)Component->GetOwner();
    auto Pawn = PlayerController->MyFortPawn;
    if (!Pawn) return ServerStopInteractWithWorkbenchActorOG(Component, ModStation);

    FGameplayTag CameraTag;
    CameraTag.TagName = UKismetStringLibrary::Conv_StringToName(L"WeaponModStation.CameraAbility.Active"); 
    FGameplayTagContainer CameraTags;
    CameraTags.GameplayTags.Add(CameraTag);
    Pawn->AbilitySystemComponent->BP_CancelAbilitiesWithTags(CameraTags);
                
    FGameplayTag InteractTag;
    InteractTag.TagName = UKismetStringLibrary::Conv_StringToName(L"Gameplay.Action.Player.InteractUse");
    FGameplayTagContainer UnblockTags;
    UnblockTags.GameplayTags.Add(InteractTag);
    Pawn->AbilitySystemComponent->BP_UnBlockAbilitiesWithTags(UnblockTags); 
    
    return ServerStopInteractWithWorkbenchActorOG(Component, ModStation);
}

// we should name these // oh and prob i need to use them but later ok
static unsigned __int64* (__fastcall* sub_13DC400)(__int64* noidea, UFortWeaponModItemDefinition* WeaponMod) = decltype(sub_13DC400)(ImageBase + 0x13DC400);
static __int64(__fastcall* sub_9BA1D68)(FFortItemEntry* ItemEntry, __int64* noidea) = decltype(sub_9BA1D68)(ImageBase + 0x9BA1D68);

void ServerPurchaseWeaponModForWeaponImplementation(UFortWeaponModStationComponent* Component, UFortWeaponModItemDefinition* WeaponMod, AFortWeapon* Weapon)
{
    auto SharedDataComponent = (UFortWeaponModStationSharedDataComponent*)GetGameState()->GetComponentByClass(UFortWeaponModStationSharedDataComponent::StaticClass());
    auto PlayerController =  (AFortPlayerControllerAthena*)Component->GetOwner();
    auto Inventory = (AFortInventory*)PlayerController->WorldInventory.ObjectPointer;
    auto GoldDef = GetAssetManager()->GameDataBR->DefaultGlobalCurrencyItemDefinition.Get();

    
    // this gets the price from the datatable
    auto ModCost = 0.f;
    for (auto Cost : SharedDataComponent->ModCostMap)
    {
        if (Cost.Second.WeaponMod == WeaponMod)
        {
            ModCost = Cost.Second.GoldCost;
            break; 
        }
    }

    TArray<FFortWeaponModSlot> PreviousModSlots;
    PreviousModSlots =  Weapon->GetWeaponModSlots();

    // so my idea is to before applying the mods store only the mods that dont interfere with the new mod
    // then we use the func that is used for spawnloot alr
    

    auto WorldItem = (UFortWorldItem*)Weapon->GetInventoryItem();

    
    // here we check for mods that use the same modslot as the mod that we are trying to apply and removing it
    for (int32 i = 0; i < Weapon->WeaponModSlots.Num(); i++)
    {
        if (Weapon->WeaponModSlots[i].WeaponMod->ModSlot.TagName == WeaponMod->ModSlot.TagName)
        {
            UE_LOG(LogWeaponModStation, Log, "Removing mod from WeaponModSlots with name: %s", Weapon->WeaponModSlots[i].WeaponMod->Name.ToString().c_str());
            WorldItem->ItemEntry.WeaponModSlots.Remove(i); // crash here too
            Weapon->WeaponModSlots.Remove(i);
            Inventory->Update(&WorldItem->ItemEntry);
            break;
        }
    }

    auto ItemEntry = Inventory->GetItemEntryByGuid(Weapon->ItemEntryGuid);

    if (!Inventory->RemoveItem(PlayerController, GoldDef, ModCost))
       return;
    
    
    FFortWeaponModSlot NewSlot;
    NewSlot.WeaponMod = WeaponMod;
    NewSlot.bIsDynamic = true;

    for (auto mod : ItemEntry->SavedWeaponModSlots)
    {
        std::cout << mod.WeaponModTemplateID.ToStr().c_str() << std::endl;
    }

    std::cout << WeaponMod->WeaponModTag.TagName.ToString().c_str() << std::endl;


    FFortSavedWeaponModSlot GoofyMod;
    GoofyMod.bIsDynamic = true;
   // GoofyMod.WeaponModTemplateID = WeaponMod.

    
    ItemEntry->WeaponModSlots.Add(NewSlot);
    //ItemEntry->SavedWeaponModSlots.Add(NewSlot);
    Weapon->WeaponModSlots.Add(NewSlot);
    Weapon->OnRep_ReplicatedWeaponModSlots(PreviousModSlots);
    
    Inventory->Update(ItemEntry);
    
}

// fix needs to be found bc when u remove a mod and then try to drop it it crashes so we need to add the default mods i managed to just replicate the bug 2 times
__int64 FortWeaponModStation::ServerPurchaseRemoveModImplementation(UFortWeaponModStationComponent* Component, UFortWeaponModItemDefinition* WeaponModToRemove, AFortWeapon* Weapon)
{
    auto PlayerController =  (AFortPlayerControllerAthena*)Component->GetOwner();
    auto Inventory = (AFortInventory*)PlayerController->WorldInventory.ObjectPointer;
    auto GoldDef = GetAssetManager()->GameDataBR->DefaultGlobalCurrencyItemDefinition.Get();
    
    Inventory->RemoveItem(PlayerController, GoldDef, (int)Component->RemoveModCost.Value);

   auto item = (UFortWorldItem*)Weapon->GetInventoryItem();

    for (int32 i = 0; i < Weapon->WeaponModSlots.Num(); i++)
    {
        if (Weapon->WeaponModSlots[i].WeaponMod == WeaponModToRemove)
        {
            item->ItemEntry.WeaponModSlots.Remove(i); //crash here
            Weapon->WeaponModSlots.Remove(i);
            break;
        }
    }
    return ServerPurchaseRemoveModImplementationOG(Component, WeaponModToRemove, Weapon);
}



void FortWeaponModStation::Init()
{
    UHook* Hook = new UHook();

    Hook->Address = ImageBase + 0xB771684;
    Hook->Detour = ServerStopInteractWithWorkbenchActor;
    Hook->Original = (void**)&ServerStopInteractWithWorkbenchActorOG;
    UKismetHookingLibrary::Hook(Hook, Address); 
 

    Hook->Address = ImageBase + 0xB770A24;
    Hook->Detour = ServerPurchaseWeaponModForWeaponImplementation;
    UKismetHookingLibrary::Hook(Hook, Address);

    Hook->Address = ImageBase + 0xB77098C; 
    Hook->Detour = ServerPurchaseRemoveModImplementation;
    Hook->Original = (void**)&ServerPurchaseRemoveModImplementationOG;
    UKismetHookingLibrary::Hook(Hook, Address);
}
