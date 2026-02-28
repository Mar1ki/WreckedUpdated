#include "pch.h"
#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/Components/FortControllerComponent_RechargeWeapons.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"

void something()
{
    auto Controllers = 	UFortKismetLibrary::GetAllFortPlayerControllers(GetWorld(), true, false);
    for (auto Controller : Controllers)
    {
        auto CurrentTime = UFortKismetLibrary::GetServerWorldTimeSeconds(GetWorld());
        auto Comp = (UFortControllerComponent_RechargeWeapons*)Controller->GetComponentByClass(UFortControllerComponent_RechargeWeapons::StaticClass());

        for (auto& NoIdea : Comp->WeaponToRechargeDataMap)
        {
            if (NoIdea.Second.ServerStartTime == -1.0f)
            {
                continue;
            }
            float CompletionTime = NoIdea.Second.ServerStartTime + (NoIdea.Second.ChargeRate * NoIdea.Second.AmountToRecharge);
            float TimeRemaining = CompletionTime - CurrentTime;
       
            if (TimeRemaining <= 0.0f)
            {
                auto Inventory = Controller->GetInventory();
                auto ItemEntry = Inventory->GetItemEntryByGuid(NoIdea.First);
                if (ItemEntry)
                {
                    auto Casted = (UFortWeaponItemDefinition*)ItemEntry->ItemDefinition;
                    auto Stats = Casted->GetStats();
                    if (ItemEntry->LoadedAmmo >= Stats->ClipSize)
                    {
                        continue;
                    }
                    ItemEntry->LoadedAmmo += NoIdea.Second.AmountToRecharge;
                    Inventory->ReplaceEntry(*ItemEntry);

                    void (*UpdateItemEntryWithServerChargeTime)(UFortControllerComponent_RechargeWeapons*, FGuid*) = decltype(UpdateItemEntryWithServerChargeTime)(ImageBase + 0x954E170);
                    NoIdea.Second.ServerStartTime = CurrentTime;
                    std::cout << "Recharge Done Starting next recharge" << std::endl;
                    std::cout << "ServerStartTime: " << NoIdea.Second.ServerStartTime << std::endl;
                    std::cout << "ChargeRate: " << NoIdea.Second.ChargeRate << std::endl;
                    UpdateItemEntryWithServerChargeTime(Comp, &NoIdea.First);
                    Comp->ClientItemStartedRecharging(NoIdea.First, CurrentTime);
                }
            }
        }
    }
}


void FortControllerComponent_RechargeWeapons::Dummy(__int64 *Component, float DeltaTime)
{

    DummyOG(Component, DeltaTime);
}
void FortControllerComponent_RechargeWeapons::Init() {
    UHook* Hook = new UHook();

    Hook->Address = 0x3C8 / 8;
    Hook->Class = UFortControllerComponent_RechargeWeapons::StaticClass();
    Hook->Detour = Dummy;
    Hook->Original = (void**)&DummyOG;
    UKismetHookingLibrary::Hook(Hook, EveryVFT); 

    delete Hook;
}