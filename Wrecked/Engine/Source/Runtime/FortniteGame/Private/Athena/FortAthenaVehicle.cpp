#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/FortAthenaVehicle.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"

void VHook(void* Base, uintptr_t Index, void* Detour, void** Original = nullptr) {
    auto VTable = (*(void***)Base);
    if (!VTable)
        return;

    if (!VTable[Index])
        return;

    if (Original)
        *Original = VTable[Index];

    DWORD dwOld;
    VirtualProtect(&VTable[Index], 8, PAGE_EXECUTE_READWRITE, &dwOld);
    VTable[Index] = Detour;
    DWORD dwTemp;
    VirtualProtect(&VTable[Index], 8, dwOld, &dwTemp);
}
/*
bool HasTireMod(IFortVehicleInterface* VehicleInterface) {
    return false;
    if (!VehicleInterface)
        return false;

    auto Vehicle = (AFortAthenaVehicle*)(__int64(VehicleInterface) - 0x510);
    if (Vehicle) {
        auto Components = Vehicle->K2_GetComponentsByClass(UFortVehicleModComponent::StaticClass());
        for (auto Component : Components) {
            auto ModComponent = (UFortVehicleModComponent*)Component;
            if (auto ModConfig = ModComponent->ModConfig) {
                if (ModConfig->ModSlot.TagName.ToString() == "Vehicle.Mod.Slot.Tire") {
                    return true;
                }
            }
        }
    }

    return false;
} */

void ApplyModToVehicleHK(IFortVehicleInterface* Vehicle, FGameplayTag* ModTag, AFortPlayerController* EventInstigator) {
    if (Vehicle && ModTag && EventInstigator) {
        UE_LOG(LogTemp, Log, "Applying Vehicle Mod with ModTag(%s)", ModTag->TagName.ToString().c_str());
        UFortVehicleModConfig* (__fastcall * GetVehicleModConfigFromTag)(IFortVehicleInterface* Vehicle, FGameplayTag* ModTag) = decltype(GetVehicleModConfigFromTag)(ImageBase + 0x9AAF080);
        auto ModConfig = GetVehicleModConfigFromTag(Vehicle, ModTag);
        void(__fastcall * ApplyToVehicleInternal)(UFortVehicleModConfig* ModConfig, IFortVehicleInterface* Vehicle) = decltype(ApplyToVehicleInternal)(ImageBase + 0xA644268);
        ApplyToVehicleInternal(ModConfig, Vehicle);
        
        if (ModConfig && ModConfig->ModSlot.TagName.ToString() == "Vehicle.Mod.Slot.Tire") {
            auto DagwoodVehicle = (AFortDagwoodVehicle*)(__int64(Vehicle) - 0x510);
            void(__fastcall * ApplyTireMod_Internal)(AFortDagwoodVehicle*) = decltype(ApplyTireMod_Internal)(ImageBase + 0x2231580);
            ApplyTireMod_Internal(DagwoodVehicle);
        }
    }
}

bool HasVehicleModApplied(IFortVehicleInterface* VehicleInterface, FGameplayTag* ModTag) {
    if (!VehicleInterface)
        return false;

    auto Vehicle = (AFortAthenaVehicle*)(__int64(VehicleInterface) - 0x510);
    if (Vehicle) {
        auto Components = Vehicle->K2_GetComponentsByClass(UFortVehicleModComponent::StaticClass());
        for (auto Component : Components) {
            auto ModComponent = (UFortVehicleModComponent*)Component;
            if (auto ModConfig = ModComponent->ModConfig) {
                if (ModConfig->ModTag == *ModTag) {
                    return true;
                }
            }
        }
    }

    return false;
}
//wtf even is this
void TryConstructWithModServer(IFortVehicleInterface* Vehicle, FGameplayTagContainer* ForceMods) {
    if (ForceMods) {
        UE_LOG(LogTemp, Log, "Applying VehicleMods!");
        for (auto Tag : ForceMods->GameplayTags) {
            Vehicle->ApplyVehicleMod(Tag, nullptr);
        }
    }
}
/*
FTireInfo* GetAppliedTireMod(AFortDagwoodVehicle* Vehicle) {
    return &Vehicle->BaseTireInfo;
    if (!Vehicle)
        return nullptr;
    auto Interface = Vehicle->GetInterface<IFortVehicleInterface>(IFortVehicleInterface::StaticClass());
    if (HasTireMod(Interface)) {
        auto VehicleConfigClass = Vehicle->VehicleConfigsClass_BR.Get();
        auto VehicleConfig = (UFortPhysicsVehicleConfigs*)VehicleConfigClass->DefaultObject;
        for (auto Pair : VehicleConfig->TireModDataMapping) {
            auto TireInfo = Pair.Second;
            return &TireInfo;
        }
        return &Vehicle->BaseTireInfo;
    }
    return &Vehicle->BaseTireInfo;
}*/

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
            New.Health = 100;
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

    UE_LOG(LogVehicles, Log, "Initializeing Vehicle Damagable Parts!");
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
    RootComp->SetPhysicsAngularVelocityInDegrees(InState.AngularVelocity, 0, FName(0));
}   

void RepairAllDamageableParts(AFortAthenaVehicle* Vehicle, float NewHealthPrecent) {
    UE_LOG(LogTemp, Log, "Repairing All DamagableParts.............");
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
    UE_LOG(LogTemp, Log, "Repairing DamagableParts.............");
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
    
/*  Hook->Address = 0x1170 / 8;
    Hook->Class = AFortAthenaVehicle::StaticClass();
    Hook->Detour = RepairAllDamageableParts;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);

    Hook->Address = 0x1160 / 8;
    Hook->Class = AFortAthenaVehicle::StaticClass();
    Hook->Detour = BreakDamageablePart;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);

    Hook->Address = 0x1168 / 8;
    Hook->Class = AFortAthenaVehicle::StaticClass();
    Hook->Detour = RepairDamageablePart;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);
    */
    
    Hook->Address = 0x520 / 8;
    Hook->Class = AFortAthenaVehicle::StaticClass();
    Hook->Detour = PostInitializeComponents;
    Hook->Original = (void**)&PostInitializeComponentsOG;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);
    
}
