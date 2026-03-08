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

        // prob most ass way btw
        if (ModTag->TagName.ToString().contains("Vehicle.Mod.Tire"))
        {
            
            auto VehicleConfigClass = Vehicle->VehicleConfigsClass_BR.Get();
            auto VehicleConfig = (UFortPhysicsVehicleConfigs*)VehicleConfigClass->DefaultObject;
            auto DagwoodVehicle = (AFortDagwoodVehicle*)Vehicle;
            DagwoodVehicle->DagwoodRuntimeModifiedProperties.TireModTag = *ModTag;

            if (auto* TireInfo = VehicleConfig->TireModDataMapping.Search([&](auto& Key, auto&) { return Key == *ModTag; }))
            {
                DagwoodVehicle->DagwoodRuntimeModifiedProperties.SpringsInfo = TireInfo->SpringTuning;
                DagwoodVehicle->DagwoodRuntimeModifiedProperties.bTireModAdjustsPosition = true;
                DagwoodVehicle->DagwoodRuntimeModifiedProperties.ServerTireModVersion++;

                /*VehicleConfig->GrassBehavior = TireInfo->GrassBehavior;
                VehicleConfig->MudBehavior = TireInfo->MudBehavior;
                VehicleConfig->RoadBehavior = TireInfo->RoadBehavior; */
                DagwoodVehicle->BaseTireInfo = *TireInfo; // that does nothing im fr or we dont onrep something
                // vehicle and base tire info


                
                auto OldModifiedProperties = DagwoodVehicle->DagwoodRuntimeModifiedProperties;
                DagwoodVehicle->OnRep_DagwoodRuntimeModifiers(OldModifiedProperties);
                DagwoodVehicle->ApplyRuntimeModifiers(); // that actually calls apply tire mod when u change TireModTag in dagwood runtime properties but doesnt do shit idk why so we just do it manually
            }
            
        }
        
    } else
    {
        UE_LOG(LogVehicles, Warning, "This vehicle has no interface or modconfig that should not happen");
    }
}

bool FortVehicleMod::CanApplyMod(IFortVehicleInterface* VehicleInterface, FGameplayTag& NewModTag)
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
    
    if (NewModTag.TagName.ToString().contains("Bulletproof") ||NewModTag.TagName.ToString().contains("Offroad") )
    {
        for (const FGameplayTag& ExistingTag : Vehicle->AppliedVehicleModTags.GameplayTags)
        {
            if (ExistingTag.TagName.ToString().contains("Bulletproof") ||  ExistingTag.TagName.ToString().contains("Offroad"))
            {
                UE_LOG(LogVehicles, Log, "Cannot apply %s: Vehicle already has a tire mod equipped.", NewModTag.TagName.ToString().c_str());
                return false;
            }
        }
    }
    
    return true;
}

//void Dummy(AFortAthenaVehicle *Vehicle, float *InOutDamage, const FGameplayTagContainer *InTags, FGameplayEffectContextHandle *EffectContextHandle)
void Dummy(AFortAthenaVehicle* Vehicle, float* InOutDamage, const FGameplayTagContainer* InTags, FGameplayEffectContextHandle *EffectContextHandle, AController* Controller, AActor* Actor, AFortPlayerPawn* PlayerPawn)
{
    std::cout << std::hex << (uint64_t)_ReturnAddress() - ImageBase << std::endl;
   /* std::cout << "InOutDamage: " << InOutDamage << std::endl;
    auto EffectContext = (FGameplayEffectContext*)EffectContextHandle->Data.Object;

    auto OBJ = (FGameplayEffectContext*)EffectContextHandle->Data.Object;
    auto HitResult = (FHitResult*)OBJ->HitResult.Object;
    if (HitResult)
    {
        HitResult.
    }
    std::cout << "dummy called" << std::endl; */
    

}
void FortVehicleMod::ServerVehicleModSeatRPC(AFortPlayerControllerAthena* PlayerController, const FGameplayTag& RequestType, UFortVehicleModComponent* ModInstance, int32 SeatIndex) {
    std::cout << "VehicleModRpc Called" <<std::endl;
    ServerVehicleModSeatRPCOG(PlayerController, RequestType, ModInstance, SeatIndex);
    
    auto Vehicle = (AFortAthenaVehicle*)PlayerController->MyFortPawn->CurrentVehicle;
    std::cout << ModInstance->Name.ToString().c_str() << std::endl;

    if (Vehicle) {
        UFortVehicleModComponent_VertexAnimTurret* Comp = Cast<UFortVehicleModComponent_VertexAnimTurret>(Vehicle->GetComponentByClass(UFortVehicleModComponent_VertexAnimTurret::StaticClass()));
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
                        auto ItemEntry = PlayerController->GetInventory()->AddItem(WeaponDef, 1, 6);
                        ItemEntry->PhantomReserveAmmo = 6;
                        auto Guid = ItemEntry->ItemGuid;
                        auto TrackerGuid = ItemEntry->TrackerGuid;
                        std::cout << WeaponDef->Name.ToString().c_str() << std::endl;
                        
                        ItemEntry->LoadedAmmo = 6; 
                        auto Weapon = (AFortWeaponRangedForVehicleMod*)PlayerController->MyFortPawn->EquipWeaponDefinition(WeaponDef, Guid, TrackerGuid, false);
                        Weapon->ModInstance = ModInstance;
                        

                        Weapon->AmmoCount = 6;
                        Weapon->OnRep_AmmoCount(6);
                        
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
    
    // vehicle mod rpc used for turrets
    Hook->Address = 0x2110 / 8;
    Hook->Class = AFortPlayerControllerAthena::StaticClass();
    Hook->Detour = ServerVehicleModSeatRPC;
    Hook->Original =(void**)&ServerVehicleModSeatRPCOG;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);
    
    // AFortAthenaVehicle::AbsorbDamageIntoDamagablePart that has no impl
   /* Hook->Address = 0x11A8/8;
    Hook->Class = AFortAthenaVehicle::StaticClass();
    Hook->Detour = Dummy;
    UKismetHookingLibrary::Hook(Hook, EveryVFT); */
}