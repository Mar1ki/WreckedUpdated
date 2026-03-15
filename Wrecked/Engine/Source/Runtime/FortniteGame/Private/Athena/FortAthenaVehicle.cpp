#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortAthenaVehicle.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Kismet/FortKismetLibrary.h"

void ApplyDamagableParts(AFortAthenaVehicle* Vehicle) {
    auto VehicleConfigClass = Vehicle->VehicleConfigsClass_BR.Get();
    TArray<FVehicleDamageablePart> Old;
    for (auto i = 0; i < Vehicle->DamageableParts.Num(); i++) {
        Old.Add(Vehicle->DamageableParts[i]);
    }
    if (VehicleConfigClass) {
        auto VehicleConfig = (UFortPhysicsVehicleConfigs*)VehicleConfigClass->DefaultObject;
        for (int i = 0; i < VehicleConfig->DamageableParts.Num(); i++) {
            auto Config = VehicleConfig->DamageableParts[i];
            FVehicleDamageablePart New;
            New.ShapeName = Config.ShapeName;
            New.ConfigIndex = i;
            New.BoneName = Config.BoneName;
            New.Health = 100; //scuffed it could be better 
            New.bConsideredCriticalHit = Config.bConsideredCriticalHit;
            Vehicle->DamageableParts.Add(New);
        }
    }
    for (auto Part : Vehicle->DamageableParts) {
        FVehicleToggleablePart New;
        New.ShapeName = Part.ShapeName;
        New.bEnabled = false;
        //Vehicle->ToggledParts.Add(New);
    } 
    Vehicle->OnRep_DamageableParts(Old);
}



static auto BGAClass = StaticFindObject<UClass>("/Script/Engine.BlueprintGeneratedClass");

AFortAthenaVehicle* SpawnBasicSUV(FVector Location, FRotator Rotation) {
    static auto VehicleClass = StaticLoadObject<UClass>("/BasicSUV/Vehicle/Valet_BasicSUV_Vehicle.Valet_BasicSUV_Vehicle_C", BGAClass);
    auto SpawnedVehicle = GetWorld()->SpawnActor<AFortAthenaVehicle>(VehicleClass, Location, Rotation);
  //  ApplyDamagableParts(SpawnedVehicle);
    return SpawnedVehicle;
}

AFortAthenaVehicle* SpawnSportsCar(FVector Location, FRotator Rotation) {
    static auto VehicleClass = StaticLoadObject<UClass>("/Valet/SportsCar/Valet_SportsCar_Vehicle.Valet_SportsCar_Vehicle_C", BGAClass);
    auto SpawnedVehicle = GetWorld()->SpawnActor<AFortAthenaVehicle>(VehicleClass, Location, Rotation);
 //  ApplyDamagableParts(SpawnedVehicle);
    return SpawnedVehicle;
}
AFortAthenaVehicle* SpawnDirtBikeVehicle(FVector Location, FRotator Rotation) {
    static auto VehicleClass = StaticLoadObject<UClass>("/Dirtbike/Vehicle/Motorcycle_DirtBike_Vehicle.Motorcycle_DirtBike_Vehicle_C", BGAClass);
    auto SpawnedVehicle = GetWorld()->SpawnActor<AFortAthenaVehicle>(VehicleClass, Location, Rotation);
    return SpawnedVehicle;
}
AFortAthenaVehicle* SpawnBossVehicle1(FVector Location, FRotator Rotation) {
    static auto VehicleClass = StaticLoadObject<UClass>("/MoonflaxBossVehicle/Gameplay/Vehicles/Valet_BossSportsCar_Moonflax_Vehicle.Valet_BossSportsCar_Moonflax_Vehicle_C", BGAClass);
    auto SpawnedVehicle = GetWorld()->SpawnActor<AFortAthenaVehicle>(VehicleClass, Location, Rotation);
    return SpawnedVehicle;
}

void FortAthenaVehicle::PostInitializeComponents(AFortAthenaVehicle* Vehicle) {
    PostInitializeComponentsOG(Vehicle);

    //UE_LOG(LogVehicles, Log, "Initializing Vehicle Damageable Parts");
    ApplyDamagableParts(Vehicle);
}
void FortAthenaVehicle::SpawnVehicles()
{
    static bool Spawned = false;
    if (!Spawned)
    {
        Spawned = true;
        TArray<AActor*> Spawners;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFortAthenaLivingWorldVehiclePointProvider::StaticClass(), &Spawners);
		static auto BasicSUVClass = StaticLoadObject<UClass>("/BasicSUV/LWSpawner/LW_BasicSUV_Spawner.LW_BasicSUV_Spawner_C", BGAClass);
        static auto SportsCarSpawner = StaticLoadObject<UClass>("/Valet/LWSpawner/SportsCar/Valet_SportsCar_LWSpawner.Valet_SportsCar_LWSpawner_C", BGAClass);
        static auto DirtBikeSpawner = StaticLoadObject<UClass>("/Dirtbike/LWSpawner/LW_DirtbikeVehicle_Spawner.LW_DirtbikeVehicle_Spawner_C", BGAClass);

        // ig we can make a check if all spawners/points are there but who the fuck cares bc its called when a playe spawns so yh
        for (auto Spawner : Spawners)
        {
            auto SpawnerLoc = Spawner->K2_GetActorLocation();
            auto SpawnerRot = Spawner->K2_GetActorRotation();
            if (Spawner->IsA(BasicSUVClass))
            {
                auto car = SpawnBasicSUV(SpawnerLoc, SpawnerRot);
            }

            if (Spawner->IsA(SportsCarSpawner))
            {
              auto car = SpawnSportsCar(SpawnerLoc, SpawnerRot);
            }
            if (Spawner->IsA(DirtBikeSpawner))
            {
                SpawnDirtBikeVehicle(SpawnerLoc, SpawnerRot);
            }
        }
        UE_LOG(LogVehicles, Log, "Spawned Vehicles");
    }
}

double UnwindDegrees(double Angle) {

    Angle = fmod(Angle, 360.0f); // rat

    if (Angle < 0.)
        Angle += 360.;

    if (Angle > 180.)
        Angle -= 360.;

    return Angle;
}

void ServerMove(AFortPhysicsPawn* Pawn, FReplicatedPhysicsPawnState InState)
{
    UPrimitiveComponent* RootComp = (UPrimitiveComponent*)Pawn->RootComponent;
	
    auto Rotation = FQuatToRot(InState.Rotation);
    
    Rotation.Yaw  = UnwindDegrees(Rotation.Yaw);
    
    Rotation.pitch = 0;
    Rotation.Roll = 0;

    RootComp->K2_SetWorldLocationAndRotation(InState.Translation, Rotation, false, nullptr, true);
    RootComp->SetPhysicsLinearVelocity(InState.LinearVelocity, 0, FName(0));
    RootComp->bComponentToWorldUpdated = true;
    RootComp->SetPhysicsAngularVelocityInDegrees(InState.AngularVelocity, 0, FName(0));
}   

void RepairAllDamageableParts(AFortAthenaVehicle* Vehicle, float NewHealthPercent) {

    if (!Vehicle) return;
    UE_LOG(LogVehicles, Warning, "NewHealthPercent: %f", NewHealthPercent);
    
    auto VehicleConfigClass = Vehicle->VehicleConfigsClass.Get();
    if (VehicleConfigClass) {
        auto VehicleConfig = (UFortPhysicsVehicleConfigs*)VehicleConfigClass->DefaultObject;

        if (Vehicle) {
            TArray<FVehicleDamageablePart> Old;
            for (auto i = 0; i < Vehicle->DamageableParts.Num(); i++) {
                Old.Add(Vehicle->DamageableParts[i]);
            }
            for (auto i = 0; i < Vehicle->DamageableParts.Num(); i++) {
                auto Found = VehicleConfig->DamageableParts.Search([&](FVehicleDamageablePartConfig& Part) {return Part.ShapeName == Vehicle->DamageableParts[i].ShapeName; });
                if (Found)
                {
                    Vehicle->DamageableParts[i].Health = Found->MaxHealth.Evaluate();
                }
            }
            Vehicle->OnRep_DamageableParts(Old);
        }
    }
}

void RepairDamageablePart(AFortAthenaVehicle* Vehicle, const FName* ShapeName, const float NewHealthPercent) {

    if (!Vehicle) return;
    UE_LOG(LogVehicles, Warning, "NewHealthPercent: %f", NewHealthPercent);
    
    auto VehicleConfigClass = Vehicle->VehicleConfigsClass.Get();
    if (VehicleConfigClass) {
        auto VehicleConfig = (UFortPhysicsVehicleConfigs*)VehicleConfigClass->DefaultObject;
        if (Vehicle && ShapeName) {
            TArray<FVehicleDamageablePart> Old;
            for (auto i = 0; i < Vehicle->DamageableParts.Num(); i++) {
                Old.Add(Vehicle->DamageableParts[i]);
            }
            for (auto i = 0; i < Vehicle->DamageableParts.Num(); i++) {
                if (Vehicle->DamageableParts[i].ShapeName == *ShapeName) {
                    auto Found = VehicleConfig->DamageableParts.Search([&](FVehicleDamageablePartConfig& Part) {return Part.ShapeName == *ShapeName; });
                    if (Found)
                    {
                        Vehicle->DamageableParts[i].Health = Found->MaxHealth.Evaluate();
                        break;
                    }
                    
                }
            }
            Vehicle->OnRep_DamageableParts(Old);
        }
    }
} 

void FortAthenaVehicle::BreakDamageablePart(AFortAthenaVehicle* Vehicle, const FName* ShapeName)
{
    if (!Vehicle) return;
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
void FortAthenaVehicle::TryGiveMetalOnHarvest(IFortVehicleInterface *Interface, AActor *DamageCauser, AController *EventInstigator, const int MetalOnHit)
{
    auto PlayerController = (AFortPlayerControllerAthena*)EventInstigator;
    if (!PlayerController) return;
    auto Pawn = PlayerController->MyFortPawn;
    if (!Pawn) return;
    auto Inventory =  (AFortInventory*)PlayerController->WorldInventory.ObjectPointer;
    auto MetalItemDefinition = GetAssetManager()->GameDataCommon->MetalItemDefinition.Get();
    auto Vehicle = (AFortAthenaVehicle*)(__int64(Interface) - 0x510);

    if (!DamageCauser->IsA(AFortWeaponPickaxeAthena::StaticClass()))return;
    auto FoundEntry = Inventory->GetItemEntryByDef(MetalItemDefinition);
    if (FoundEntry)
    {

        if (FoundEntry->Count == 500)
        {
            auto Pickup = FortKismetLibrary::SpawnPickup(DamageCauser->K2_GetActorLocation(), MetalItemDefinition, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::TossedByPlayer, MetalOnHit, 0, Pawn);
            Pickup->SetCombinePickupsWhenTossCompletes(true);
            
        } else
        {
            FoundEntry->Count += MetalOnHit;
            Inventory->ReplaceEntry(*FoundEntry);
            PlayerController->ClientReportGotResourceFromVehicle(Vehicle, MetalOnHit);

        }
    } else
    {
        Inventory->AddItem(MetalItemDefinition, MetalOnHit, 0);
        PlayerController->ClientReportGotResourceFromVehicle(Vehicle, MetalOnHit);
    }
}

void FortAthenaVehicle::Hook()
{
    UHook* Hook = new UHook();
    
   // ServerMove Reliable or smth like that
    Hook->Address = 0x113;
    Hook->Class = AFortPhysicsPawn::StaticClass();
    Hook->Detour = ServerMove;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);
    // ServerMove
    Hook->Address = 0x112;
    Hook->Class = AFortPhysicsPawn::StaticClass();
    Hook->Detour = ServerMove;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);

    // break damagablepart
    Hook->Address = 0x1160 / 8;
    Hook->Class = AFortAthenaVehicle::StaticClass();
    Hook->Detour = BreakDamageablePart;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);
    
    Hook->Address = 0x1170 / 8;
    Hook->Class = AFortAthenaVehicle::StaticClass();
    Hook->Detour = RepairAllDamageableParts;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);

    Hook->Address = 0x1168 / 8;
    Hook->Class = AFortAthenaVehicle::StaticClass();
    Hook->Detour = RepairDamageablePart;
    UKismetHookingLibrary::Hook(Hook, EveryVFT); 
    
    Hook->Address = 0x520 / 8;
    Hook->Class = AFortAthenaVehicle::StaticClass();
    Hook->Detour = PostInitializeComponents;
    Hook->Original = (void**)&PostInitializeComponentsOG;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);
    
    Hook->Address = 0x748 / 8;
    Hook->Class = AFortAthenaVehicle::StaticClass();
    Hook->Interface = IFortVehicleInterface::StaticClass();
    Hook->Detour = TryGiveMetalOnHarvest;
    UKismetHookingLibrary::Hook(Hook, Interface);
    
}
