#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Weapons/Projectiles/FortProjectileAthena.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"

void FortProjectileAthena::ServerNotifyPawnHit(
    AFortProjectileAthena* projectile,
    FHitResult& Hit,
    const TArray<uint8>& Context)
{
    ServerNotifyPawnHitOG(projectile, Hit, Context);

    auto Owner = projectile->GetOwnerPlayerController();
    auto OwnerWeapon = (AFortWeaponRanged*)projectile->GetOwnerWeapon();
    auto Component = Hit.Component.Get();
    if (!Component) return;

    if (!projectile) return;
    auto Actor = Component->GetOwner();
    if (!Actor) return;

    auto WeaponStats = OwnerWeapon->WeaponData->GetStats();
    if (!WeaponStats) return;

    if (Actor->IsActorBeingDestroyed()) return; // ig that can help with the crash
    if (Actor->bCanBeDamaged != 1) return; // random crash here

    auto SendCue = [&](float Damage) {
        FAthenaBatchedDamageGameplayCues_Shared SharedCue{};
        SharedCue.Location = (FVector_NetQuantize10)Hit.ImpactPoint;
        SharedCue.Normal = Hit.ImpactNormal;
        SharedCue.Magnitude = Damage;
        SharedCue.bWeaponActivate = true;
        SharedCue.bIsBallistic = true;
        SharedCue.bIsValid = true;

        FAthenaBatchedDamageGameplayCues_NonShared NonSharedCue{};
        NonSharedCue.HitActor = Actor;

        Owner->MyFortPawn->NetMulticast_Athena_BatchedDamageCues(SharedCue, NonSharedCue);
    };
    
    auto SendCueNew = [&](float Damage) {
        if (Actor->IsA(AFortPlayerPawn::StaticClass()))
        {
            float Multiplier = (Hit.BoneName.ToString() == "head") ? WeaponStats->DamageZone_Critical : 1.f;
            Damage = WeaponStats->DmgPB * Multiplier;
        }
        FAthenaBatchedDamageGameplayCues_Shared SharedCue{};
        SharedCue.Location = (FVector_NetQuantize10)Hit.ImpactPoint;
        SharedCue.Normal = Hit.ImpactNormal;
        SharedCue.Magnitude = Damage;
        SharedCue.bWeaponActivate = true;
        SharedCue.bIsBallistic = true;
        if (Actor->IsA(AFortPlayerPawn::StaticClass()))
        {
            auto Pawn = (AFortPlayerPawn*)Actor;
            auto Shield = Pawn->GetShield();
            SharedCue.bIsCritical = Hit.BoneName.ToString() == "head";
            SharedCue.bIsShield = Shield > 0;
            SharedCue.bIsShieldDestroyed = Shield > 0 && Shield <= Damage;
        }
        SharedCue.bIsValid = true;

        FAthenaBatchedDamageGameplayCues_NonShared NonSharedCue{};
        NonSharedCue.HitActor = Actor;

        Owner->MyFortPawn->NetMulticast_Athena_BatchedDamageCues(SharedCue, NonSharedCue);
    };

    if (bUseExperimentalDamaging)
    {
           auto a =  (UFortMovementComp_Character*)Owner->MyFortPawn->CharacterMovement;
            SendCueNew(WeaponStats->EnvDmgPB);
            UGameplayStatics::ApplyPointDamage(Actor, WeaponStats->EnvDmgPB, Hit.Location, Hit, Owner, OwnerWeapon, a->ImpactDamageTypeClass);
    } else
    {
        if (auto BuildingActor = Cast<ABuildingSMActor>(Actor)) {
            float Damage = WeaponStats->EnvDmgPB;
            SendCue(Damage);

            float HealthLeft = BuildingActor->GetHealth() - Damage;
            BuildingActor->SetHealth(HealthLeft);
            BuildingActor->ForceNetUpdate();
            if (HealthLeft <= 0)
                BuildingActor->K2_DestroyActor();
        }

        if (Actor->Class->IsA(UBlueprintGeneratedClass::StaticClass()) &&
            Actor->GetName().contains("HealingCactus"))
        {
            auto Function = Actor->Class->GetFunction("BGA_Moonflax_HealingCactus_C", "StartBurst");
            struct StartBurstParams { bool VehicleCaused; };
            StartBurstParams params{ false };
            Actor->ProcessEvent(Function, &params);
        }

        if (auto Vehicle = Cast<AFortAthenaVehicle>(Actor)) {
            float Damage = WeaponStats->EnvDmgPB;
            FVector BoneLocation;
            auto BoneName = Vehicle->SkeletalMesh->FindClosestBone_K2(Hit.ImpactPoint, &BoneLocation, 0, false);
            if (BoneName != NAME_None) {
                TArray<FVehicleDamageablePart> Old;
                Old.Append(Vehicle->DamageableParts);

                for (int i = 0; i < Vehicle->DamageableParts.Num(); i++) {
                    auto& Part = Vehicle->DamageableParts[i];
                    if (Part.BoneName == BoneName) {
                        Part.Health = FMath::Max(0.f, Part.Health - Damage);
                        break;
                    }
                }
                Vehicle->OnRep_DamageableParts(Old);
            }

            SendCue(Damage);

            Vehicle->HealthSet->Health.CurrentValue -= Damage;
            Vehicle->OnRep_HealthSet();
            if (Vehicle->HealthSet->Health.CurrentValue <= 0)
                Vehicle->DestroyVehicle();
        }

        // ModBoxes and CowCatcher mod
        if (Actor->GetName().contains("ValetMod") || Actor->GetName().contains("ModBox")) {
            if (Actor->GetFullName().contains("Mod")) {
                float Damage = WeaponStats->EnvDmgPB;
                SendCue(Damage);

                auto ModActor = (ABuildingActor*)Actor;
                float HealthLeft = ModActor->GetHealth() - Damage;
                ModActor->SetHealth(HealthLeft);
                if (HealthLeft <= 0)
                    ModActor->K2_DestroyActor();
            }
        }

        if (auto SuperDingo = Cast<AAthenaSuperDingo>(Actor)) {
            float Damage = WeaponStats->EnvDmgPB;
            SendCue(Damage);

            float HealthLeft = SuperDingo->GetHealth() - Damage;
            SuperDingo->SetHealth(HealthLeft);
            if (HealthLeft <= 0)
                SuperDingo->K2_DestroyActor();
        }

        if (auto CrashPad = Cast<ABuildingGameplayActorCrashpad>(Actor)) {
            CrashPad->DestroyCrashpad();
            /* float Damage = WeaponStats->EnvDmgPB;
             SendCue(Damage);
     
             float HealthLeft = CrashPad->GetHealth() - Damage;
             CrashPad->SetHealth(HealthLeft);
             if (HealthLeft <= 0)
                 CrashPad->K2_DestroyActor();*/
        }

        if (auto Balloon = Cast<ABuildingGameplayActorBalloon>(Actor)) {
            float Damage = WeaponStats->EnvDmgPB;
            SendCue(Damage);

            float HealthLeft = Balloon->GetHealth() - Damage;
            Balloon->SetHealth(HealthLeft);
            if (HealthLeft <= 0)
                Balloon->K2_DestroyActor();
        }

        if (auto TargetPawn = Cast<AFortPlayerPawn>(Actor)) {
            float Multiplier = (Hit.BoneName.ToString() == "head")
                ? WeaponStats->DamageZone_Critical
                : 1.f;

            float Damage = WeaponStats->DmgPB * Multiplier;

            FAthenaBatchedDamageGameplayCues_Shared SharedCue{};
            SharedCue.Location = (FVector_NetQuantize10)Hit.ImpactPoint;
            SharedCue.Normal = Hit.ImpactNormal;
            SharedCue.Magnitude = Damage;
            SharedCue.bWeaponActivate = true;
            SharedCue.bIsCritical = Hit.BoneName.ToString() == "head";
            SharedCue.bIsBallistic = true;
            SharedCue.bIsValid = true;

            FAthenaBatchedDamageGameplayCues_NonShared NonShared{};
            NonShared.HitActor = TargetPawn;

            if (UFortGameStateComponent_BattleRoyaleGamePhaseLogic::Get(GetWorld())->GamePhase != EAthenaGamePhase::Warmup) {
                float Shield = TargetPawn->GetShield();
                float Health = TargetPawn->GetHealth();

                if (Shield > 0) {
                    SharedCue.bIsShield = true;
                    if (Shield < Damage) {
                        SharedCue.bIsShieldDestroyed = true;
                        float Remain = Damage - Shield;
                        TargetPawn->SetShield(0);
                        TargetPawn->SetHealth(Health - Remain);
                    } else {
                        TargetPawn->SetShield(Shield - Damage);
                    }
                } else if (Health > Damage) {
                    TargetPawn->SetHealth(Health - Damage);
                } else {
                    TargetPawn->ForceKill(OwnerWeapon->WeaponData->GameplayTags.GameplayTags[0], Owner, OwnerWeapon);
                }

                TargetPawn->ForceNetUpdate();
            }

            Owner->MyFortPawn->NetMulticast_Athena_BatchedDamageCues(SharedCue, NonShared);
        }
    }
}

void FortProjectileAthena::Init() {
	UHook* Hook = new UHook();

	Hook->Address = ImageBase + 0x910F4D4;
	Hook->Detour = ServerNotifyPawnHit;
	Hook->Original = (void**)&ServerNotifyPawnHitOG;
	UKismetHookingLibrary::Hook(Hook, EHook::Address);

	delete Hook;
}