#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortVehicleMod.h"

#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"



// not used rn will delete after lwm spawning
UClass* FortVehicleMod::GetRandomVehicleMod()
{
    static const char* ModPaths[] = {
        "/VehicleMods/Mods/SpikedBumper/VMConfig_FrontBumper_Spiked.VMConfig_FrontBumper_Spiked_C", // spiked cow catcher bro
        "/VehicleMods/Mods/MachineGunMod/VMConfig_MachineGun_Base.VMConfig_MachineGun_Base_C", // machine gun bro
        "/VehicleMods/Mods/Mortar/VMConfig_Mortar_Base.VMConfig_Mortar_Base_C", // grenade turret
        "/CowCatcherMod/Mods/CowCatcher/VehicleModConfig_CowCatcherSmall.VehicleModConfig_CowCatcherSmall_C", // cow catcher
        "/VehicleMods/Mods/SocketPropDestructor/Tire/VMConfig_SocketPropDestructor_Tire.uasset"
        // um these 3 mods like window is for the boss car so it "works" on every other car and these sockets idk that needs to be tested
       // "/VehicleMods/Mods/Window/ArmoredWindow/VMConfig_Window_Armored.VMConfig_Window_Armored_C", 
       // "/VehicleMods/Mods/SocketPropDestructor/Tire/VMConfig_SocketPropDestructor_Tire.VMConfig_SocketPropDestructor_Tire_C",
       // "/VehicleMods/Mods/SocketPropDestructor/Turret/VMConfig_SocketPropDestructor_Turret.VMConfig_SocketPropDestructor_Turret_C",
    };
    // idk that math works and like idc if its unproper
    static bool Seeded = false;
    if (!Seeded)
    {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        Seeded = true;
    }

    int PathCount = sizeof(ModPaths) / sizeof(ModPaths[0]);
    int Index = std::rand() % PathCount;
    return StaticLoadObject<UClass>(ModPaths[Index]);
}


void ApplyVehicleModImplementation(IFortVehicleInterface* VehicleInterface, FGameplayTag* ModTag, AFortPlayerController* EventInstigator)
{
    auto ModConfig = FortVehicleMod::GetVehicleModConfigFromTagInternal(VehicleInterface, ModTag);
    if (ModConfig && VehicleInterface)
    {
        auto Vehicle = (AFortAthenaVehicle*)(__int64(VehicleInterface) - 0x510);
        FGameplayTag Tag;   
        Tag.TagName = ModConfig->ModTag.TagName;
        Vehicle->AppliedVehicleModTags.GameplayTags.Add(Tag);
        FortVehicleMod::ApplyModConfigToVehicleInternal(ModConfig, VehicleInterface);
        
        auto idk = (AFortDagwoodVehicle*)Vehicle;
        idk->DagwoodRuntimeModifiedProperties.TireModTag = *ModTag;
        auto old = idk->DagwoodRuntimeModifiedProperties;
        idk->OnRep_DagwoodRuntimeModifiers(old);
        idk->ApplyRuntimeModifiers(); //we love it
    } else
    {
        UE_LOG(LogVehicles, Warning, "This vehicle has no interface or modconfig that should not happen");
    }
}

bool CanApplyMod(IFortVehicleInterface* VehicleInterface, FGameplayTag& NewModTag)
{
    auto Vehicle = (AFortAthenaVehicle*)(__int64(VehicleInterface) - 0x510);
    
    // spiked bumper and cow catcher detection
    if (NewModTag.TagName.ToString().contains("FrontBumper") ||NewModTag.TagName.ToString().contains("Cowcatcher") )
    {
        for (const FGameplayTag& ExistingTag : Vehicle->AppliedVehicleModTags.GameplayTags)
        {
            if (ExistingTag.TagName.ToString().contains("FrontBumper") ||  ExistingTag.TagName.ToString().contains("Cowcatcher"))
            {
                UE_LOG(LogVehicles, Log, "Cannot apply %s: Vehicle already has a bumper mod equipped.", NewModTag.TagName.ToString().c_str());
                return false;
            }
        }
    }

    if (NewModTag.TagName.ToString().contains("Mortar") ||NewModTag.TagName.ToString().contains("MachineGun") )
    {
        for (const FGameplayTag& ExistingTag : Vehicle->AppliedVehicleModTags.GameplayTags)
        {
            if (ExistingTag.TagName.ToString().contains("Mortar") ||  ExistingTag.TagName.ToString().contains("MachineGun"))
            {
                UE_LOG(LogVehicles, Log, "Cannot apply %s: Vehicle already has a weapon mod equipped.", NewModTag.TagName.ToString().c_str());
                return false;
            }
        }
    }
    
    return true;
}

void Dummy(AFortAthenaVehicle *Vehicle, float *InOutDamage, const FGameplayTagContainer *InTags, const FGameplayEffectContextHandle *EffectContext)
{
    std::cout << "InOutDamage: " << InOutDamage << std::endl;
    for (auto tag : InTags->GameplayTags)
    {
        std::cout << "Tag: "<< tag.TagName.ToString().c_str() << std::endl;
    }
    
    std::cout << "dummy called" << std::endl;
    

}
void FortVehicleMod::ServerVehicleModSeatRPC(AFortPlayerControllerAthena* PlayerController, const FGameplayTag& RequestType, UFortVehicleModComponent* ModInstance, int32 SeatIndex) {
    std::cout << "Vehicle Mod Rpc Called" << std::endl;
    ServerVehicleModSeatRPCOG(PlayerController, RequestType, ModInstance, SeatIndex);
    
    AFortAthenaVehicle* Car = Cast<AFortAthenaVehicle>(PlayerController->MyFortPawn->CurrentVehicle);
    
    if (Car) {
        UFortVehicleModComponent_VertexAnimTurret* Comp = Cast<UFortVehicleModComponent_VertexAnimTurret>(Car->GetComponentByClass(UFortVehicleModComponent_VertexAnimTurret::StaticClass()));
        if (Comp) {
            for (UFortVehicleSeatWeaponComponent* SeatComp : Comp->DynamicSeatWeaponComponents) {
                for (FWeaponSeatDefinition& Weapon : SeatComp->WeaponSeatDefinitions) {
                    if (Weapon.SeatIndex != SeatIndex)
                        continue;

                    UFortWeaponItemDefinition* WeaponDef = Weapon.VehicleWeapon;
                    auto Item = PlayerController->GetInventory()->Inventory.ReplicatedEntries.Search([WeaponDef](FFortItemEntry& Entry) {return Entry.ItemDefinition == WeaponDef; });
                    if (Item)
                    {
                        // we need to actually remove the wepaon its not removed
                        UE_LOG(LogTemp, Log, "Removing VehicleWeapon!");
                        PlayerController->GetInventory()->Remove(Item->ItemGuid);
                        PlayerController->ServerExecuteInventoryItem(PlayerController->GetInventory()->Inventory.ReplicatedEntries[PlayerController->GetInventory()->Inventory.ReplicatedEntries.Num() - 1].ItemGuid);
                        Comp->AuthorityTryUnequipModSeatWeapon(SeatIndex);
                    }
                    else {
                        UE_LOG(LogTemp, Log, "Adding VehicleWeapon!");
                        auto ItemEntry = PlayerController->GetInventory()->AddItem(WeaponDef, 1, 100);
                        ItemEntry->PhantomReserveAmmo = 100;
                        auto Guid = ItemEntry->ItemGuid;
                        auto TrackerGuid = ItemEntry->TrackerGuid;
                        std::cout << WeaponDef->Name.ToString().c_str() << std::endl;
                        
                        ItemEntry->LoadedAmmo = 5; 
                        auto Weapon = (AFortWeaponRangedForVehicleMod*)PlayerController->MyFortPawn->EquipWeaponDefinition(WeaponDef, Guid, TrackerGuid, false);

                        Weapon->AmmoCount = 5;
                        Weapon->OnRep_AmmoCount(0);
                        
                        Weapon->bIsEquippingWeapon = false;
                        Weapon->OnRep_IsEquippingWeapon();
                        
                        Comp->AuthorityTryEquipModSeatWeapon(SeatIndex);
                        Weapon->TryToCustomFire();
                    }
                }
            }
        }
    }
}



void FortVehicleMod::BreakDamagablePart(AFortAthenaVehicle* Vehicle, const FName* ShapeName)
{
    auto OldToggledParts = Vehicle->ToggledParts;
    auto OldDamageableParts = Vehicle->DamageableParts;
    for (auto& Part : Vehicle->DamageableParts)
    {
        if (Part.ShapeName == *ShapeName)
        {
            FVehicleToggleablePart DamagedPart;
            DamagedPart.ShapeName = *ShapeName;
            DamagedPart.bEnabled = true;
            Vehicle->ToggledParts.Add(DamagedPart);
            Part.Health = 0;
            
            Vehicle->OnRep_DamageableParts(OldDamageableParts);
            Vehicle->OnRep_ToggleableParts(OldToggledParts);
            break;
        }
    }
}
void FortVehicleMod::Init()
{
    
    UHook* Hook = new UHook();
    
    // ApplyMod
    Hook->Class = AFortAthenaVehicle::StaticClass();
    Hook->Interface = IFortVehicleInterface::StaticClass();
    Hook->Address = 0x1A8 / 8;
    Hook->Detour = ApplyVehicleModImplementation;
    UKismetHookingLibrary::Hook(Hook, Interface);

    // can apply mod thing 
    Hook->Class = AFortAthenaVehicle::StaticClass();
    Hook->Interface = IFortVehicleInterface::StaticClass();
    Hook->Address = 0x1A0 / 8;
    Hook->Detour = CanApplyMod;
    UKismetHookingLibrary::Hook(Hook, Interface);
    
    // break damagablepart
    Hook->Address = 0x1160 / 8;
    Hook->Class = AFortAthenaVehicle::StaticClass();
    Hook->Detour = BreakDamagablePart;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);


    // vehicle mod rpc used for turrets
    Hook->Address = 0x2110 / 8;
    Hook->Class = AFortPlayerControllerAthena::StaticClass();
    Hook->Detour = ServerVehicleModSeatRPC;
    Hook->Original =(void**)&ServerVehicleModSeatRPCOG;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);
    
    // AFortAthenaVehicle::AbsorbDamageIntoDamagablePart that has no impl
    Hook->Address = 0x11A8/8;
    Hook->Class = AFortAthenaVehicle::StaticClass();
    Hook->Detour = Dummy;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);
}