#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortVehicleMod.h"

#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"

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
            auto DagwoodVehicle = (AFortDagwoodVehicle*)Vehicle;
            auto DagwoodConfig = DagwoodVehicle->FortDagwoodVehicleConfigs;
            DagwoodVehicle->DagwoodRuntimeModifiedProperties.TireModTag = *ModTag;

            if (auto* TireInfo = DagwoodConfig->TireModDataMapping.Search([&](auto& Key, auto&) { return Key == *ModTag; }))
            {
                DagwoodVehicle->DagwoodRuntimeModifiedProperties.SpringsInfo = TireInfo->SpringTuning;
                DagwoodVehicle->DagwoodRuntimeModifiedProperties.bTireModAdjustsPosition = true;
                DagwoodVehicle->DagwoodRuntimeModifiedProperties.ServerTireModVersion++;
                DagwoodConfig->GrassBehavior = TireInfo->GrassBehavior;
                DagwoodConfig->MudBehavior = TireInfo->MudBehavior;
                DagwoodConfig->RoadBehavior = TireInfo->RoadBehavior;
                DagwoodVehicle->OnRep_VehicleAttributes(); // maybe
                DagwoodVehicle->ApplyRuntimeModifiers();
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
void Dummy(AFortAthenaVehicle* Vehicle, float* InOutDamage, const FGameplayTagContainer* InTags, const FGameplayEffectContextHandle *EffectContextHandle)
{
    std::cout << std::hex << (uint64_t)_ReturnAddress() - ImageBase << std::endl;
    auto Object = EffectContextHandle->Data.Object;
    auto hitresult = Object->HitResult.Object;
    UGameplayStatics::ApplyDamage((AActor*)hitresult->Component.Get(), *InOutDamage, nullptr, nullptr, nullptr);

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
bool HasTireMod(IFortVehicleInterface* Interface)
{
    auto Vehicle = (AFortAthenaVehicle*)(__int64(Interface) - 0x510);

    for (const FGameplayTag& ExistingTag : Vehicle->AppliedVehicleModTags.GameplayTags)
    {
        if (ExistingTag.TagName.ToString().contains("Tire"))
        {
            return true;
        }
    }
    return false;
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
   /* Hook->Address = 0x2110 / 8;
    Hook->Class = AFortPlayerControllerAthena::StaticClass();
    Hook->Detour = ServerVehicleModSeatRPC;
    Hook->Original =(void**)&ServerVehicleModSeatRPCOG;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);*/
    
    // AFortAthenaVehicle::AbsorbDamageIntoDamagablePart that has no impl
    // Hook->Address = 0x11A8/8;
    // Hook->Class = AFortAthenaVehicle::StaticClass();
    // Hook->Detour = Dummy;
    // UKismetHookingLibrary::Hook(Hook, EveryVFT); 

    Hook->Class = AFortAthenaVehicle::StaticClass();
    Hook->Interface = IFortVehicleInterface::StaticClass();
    Hook->Address = 0x1D0 / 8;
    Hook->Detour = HasTireMod;
    UKismetHookingLibrary::Hook(Hook, Interface);
}