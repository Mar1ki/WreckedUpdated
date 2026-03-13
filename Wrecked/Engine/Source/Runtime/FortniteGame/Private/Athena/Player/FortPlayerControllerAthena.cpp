#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/Player/FortPlayerControllerAthena.h"

#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Plugins/LateGameLibrary/Public/LateGameLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Player/FortPlayerController.h"


void FortPlayerControllerAthena::ServerAttemptAircraftJump(UFortControllerComponent_Aircraft* Comp, FRotator ClientRot)
{
    auto Controller = (AFortPlayerControllerAthena*)Comp->GetOwner();
    auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;

    // TODO: add ServerRemoveAllItems func ( i think its what its called i forget )
	TArray<FGuid> ItemsToRemove;
    auto Inventory = (AFortInventory*)Controller->WorldInventory.GetObjectRef();
	for (const auto& Item : Inventory->Inventory.ItemInstances)
	{
		if (Item && Item->CanBeDropped())
			ItemsToRemove.Add(Item->ItemEntry.ItemGuid);
	}

	for (const FGuid& Guid : ItemsToRemove)
		Inventory->Remove(Guid);
    
    GameMode->RestartPlayer(Controller);
    if (Controller->MyFortPawn)
    {
        static auto Effect = StaticLoadObject<UClass>("/Game/Athena/SafeZone/GE_OutsideSafeZoneDamage.GE_OutsideSafeZoneDamage_C");
        auto PlayerState = (AFortPlayerStateAthena*)Controller->PlayerState;

        bool Found = false;
        for (auto ActiveEffect : PlayerState->AbilitySystemComponent->ActiveGameplayEffects.GameplayEffects_Internal) {
            auto Handle = *(FActiveGameplayEffectHandle*)(__int64(&ActiveEffect) + 0x10);

            if (ActiveEffect.Spec.Def) {
                if (ActiveEffect.Spec.Def->IsA(Effect)) {
                    PlayerState->AbilitySystemComponent->UpdateActiveGameplayEffectSetByCallerMagnitude(Handle,
                        FGameplayTag(UKismetStringLibrary::Conv_StringToName(L"SetByCaller.StormCampingDamage")), 1);
                    Found = true;
                    return;
                }
            }
        }
        
        if (!Found)
        {
            auto EffectHandle = FGameplayEffectContextHandle();
            auto SpecHandle = PlayerState->AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(Effect, 0, EffectHandle);
            PlayerState->AbilitySystemComponent->UpdateActiveGameplayEffectSetByCallerMagnitude(SpecHandle,
                FGameplayTag(UKismetStringLibrary::Conv_StringToName(L"SetByCaller.StormCampingDamage")), 1);
        }

        Controller->MyFortPawn->bIsInAnyStorm = false; //Same for respawning!
        Controller->MyFortPawn->OnRep_IsInAnyStorm();
        Controller->MyFortPawn->bIsInsideSafeZone = true;
        Controller->MyFortPawn->OnRep_IsInsideSafeZone();
        
        Controller->ClientSetRotation(ClientRot, false);
        Controller->MyFortPawn->ParachuteDeployTraceForGroundDistance.Value = 10000.0f;
        Controller->MyFortPawn->ParachuteDeployTraceForGroundDistance.Curve.CurveTable = nullptr;
        Controller->MyFortPawn->ParachuteDeployTraceForGroundDistance.Curve.RowName = FName();
        Controller->MyFortPawn->BeginSkydiving(true);

        if (bLateGame)
        {
            Controller->MyFortPawn->SetShield(100);
            ULateGameLibrary::GiveRandomLoadoutToPlayer(Controller);
        }
        // we should remove that prob
        ForEachMutator<AFortAthenaMutator_InventoryOverride>(
            GetWorld(), 
            AFortAthenaMutator_InventoryOverride::StaticClass(),
            [&](AFortAthenaMutator_InventoryOverride* Mutator)
            {
                const auto& Loadout = Mutator->InventoryLoadouts[std::rand() % Mutator->InventoryLoadouts.Num()];
                for (auto& ItemAndCount : Loadout.LoadoutList)
                {
                    int32 Ammo = 0;
                    if (auto* WeaponDef = Cast<UFortWeaponItemDefinition>(ItemAndCount.Item))
                        if (WeaponDef->GetStats())
                            Ammo = WeaponDef->GetStats()->ClipSize;
                    Inventory->AddItem((UFortItemDefinition*)ItemAndCount.Item, ItemAndCount.Count.Evaluate(), Ammo);
                }
                
                auto AddResource = [&](EFortResourceType Type) {
                    Inventory->AddItem(UFortKismetLibrary::K2_GetResourceItemDefinition(Type), 500, 0);
                };
                AddResource(EFortResourceType::Wood);
                AddResource(EFortResourceType::Stone);
                AddResource(EFortResourceType::Metal);
            }
        );
    }
}

void ServerReturnToMainMenu(UObject* Context, FFrame& Stack) {
    auto PlayerController = static_cast<AFortPlayerControllerAthena*>(Context);
    Stack.IncrementCode();
    if (!PlayerController) return;
    PlayerController->ServerSuicide();
    PlayerController->ClientReturnToMainMenuWithTextReason(UKismetTextLibrary::Conv_StringToText(L""));
    PlayerController->ClientEndOnlineSession();
  //  PlayerController->ClientEndGameKick();
    if (PlayerController->MyFortPawn)
        PlayerController->MyFortPawn->KillDisconnectedPawn();
}

void FortPlayerControllerAthena::ClientIsReadyToRespawn(AFortPlayerControllerAthena* PlayerController) {
    ClientIsReadyToRespawnOG(PlayerController);
    AFortPlayerStateAthena* PlayerStateAthena = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);
    PlayerController->GetInventory()->Inventory.ReplicatedEntries.Free();
    PlayerController->GetInventory()->Inventory.ItemInstances.Free();
    PlayerController->GetInventory()->Inventory.MarkArrayDirty();
    if (GetGameState()->IsRespawningAllowed(PlayerStateAthena))
    {
        FFortRespawnData RespawnData = PlayerStateAthena->RespawnData;

        FTransform SpawnTransform{};
        SpawnTransform.Translation = RespawnData.RespawnLocation;
        SpawnTransform.Scale3D = FVector(1, 1, 1);

        auto Pawn = (AFortPlayerPawnAthena*)GetGameMode()->SpawnDefaultPawnAtTransform(PlayerController, SpawnTransform);
        PlayerController->Possess(Pawn);
        PlayerController->RespawnPlayerAfterDeath(true);

        Pawn->bIsInAnyStorm = false;
        Pawn->bIsInsideSafeZone = true;
        Pawn->OnRep_IsInAnyStorm();
        Pawn->OnRep_IsInsideSafeZone();

        Pawn->SetHealth(100);
        Pawn->SetShield(0);
        PlayerStateAthena->RespawnData.bClientIsReady = true;

        AFortInventory* Inventory = (AFortInventory*)PlayerController->WorldInventory.GetObjectRef();
        for (int i = 0; i < GetGameMode()->StartingItems.Num(); i++)
        {
            auto Item = GetGameMode()->StartingItems[i];
            if (Item.Count <= 0) continue;

            if (!Item.Item || (Item.Item && Item.Item->IsA(UFortResourceItemDefinition::StaticClass())))
                continue;
            if (Item.Item->IsA(UFortSmartBuildingItemDefinition::StaticClass()))
                continue;
            if (!Item.Item->IsA(UFortItemDefinition::StaticClass()))
                continue;

            Inventory->AddItem((UFortItemDefinition*)Item.Item, Item.Count, 0);
        }

        Inventory->AddItem(PlayerController->CosmeticLoadoutPC.Pickaxe->WeaponDefinition, 1, 0);
    }
}


void ServerGiveCreativeItem(UObject* Context, FFrame& Stack) {
    auto& CreativeItem = Stack.StepCompiledInRef<FFortItemEntry>();
    auto& ItemToRemoveGuid = Stack.StepCompiledInRef<FGuid>();
    Stack.IncrementCode();

    auto PlayerController = static_cast<AFortPlayerControllerAthena*>(Context);
    auto Inventory = (AFortInventory*)PlayerController->WorldInventory.ObjectPointer;
    std::cout << CreativeItem.WeaponModSlots.Num() << std::endl;
    std::cout << CreativeItem.SavedWeaponModSlots.Num() << std::endl;
    
    
    Inventory->Remove(ItemToRemoveGuid);
    auto itemdef = (UFortItemDefinition*)CreativeItem.ItemDefinition;
    Inventory->AddItem(itemdef, CreativeItem.Count, 10, CreativeItem.Level);

}

void FortPlayerControllerAthena::GetPlayerViewPoint(APlayerController* PlayerController, FVector& Loc, FRotator& Rot)
{
    if (PlayerController->StateName == UKismetStringLibrary::Conv_StringToName(L"Spectating"))
    {
        //std::cout << "Spectating bro" << std::endl;
    } else
    {
        GetPlayerViewPointOG(PlayerController, Loc, Rot);
    }
}

void FortPlayerControllerAthena::Init()
{
    UHook* Hook = new UHook();
    
    Hook->Path = "/Script/FortniteGame.FortPlayerController.ServerReturnToMainMenu";
    Hook->Detour = ServerReturnToMainMenu;
    UKismetHookingLibrary::Hook(Hook, Exec);
    
    Hook->Address = 0xa7;
    Hook->Class = UFortControllerComponent_Aircraft::StaticClass();
    Hook->Detour = ServerAttemptAircraftJump;
    UKismetHookingLibrary::Hook(Hook, VFT);
    

    // TODO: ServerRestartPlayer is unstripped btw on AFortPlayerControllerZone, makes this func auto.
    /*
    Hook->Address = 0x2C48 / 8; 
    Hook->Class = AFortPlayerControllerAthena::StaticClass();
    Hook->Detour = ClientIsReadyToRespawn;
    Hook->Original = (void**)&ClientIsReadyToRespawnOG;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);
    */

    Hook->Path = "/Script/FortniteGame.FortPlayerControllerAthena.ServerGiveCreativeItem";
    Hook->Detour = ServerGiveCreativeItem;
    UKismetHookingLibrary::Hook(Hook, Exec);
    delete Hook;

    Hook->Address = 0x7F8/8;
    Hook->Detour = GetPlayerViewPoint;
    Hook->Original = (void**)&GetPlayerViewPointOG;
    Hook->Class = AFortPlayerControllerAthena::StaticClass();
    UKismetHookingLibrary::Hook(Hook, EveryVFT);
}