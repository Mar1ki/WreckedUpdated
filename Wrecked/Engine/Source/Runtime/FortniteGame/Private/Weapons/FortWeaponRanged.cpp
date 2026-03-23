#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Weapons/FortWeaponRanged.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"

void ServerNotifyPawnHit_Implemenation(AFortWeaponRanged* Weapon, const struct FHitResult& Hit, const struct FVector& ProjectileOriginPosition, float ProjectileStartTimestamp, const TArray<uint8>& Context, const struct FVector& LocalSpaceImpactPoint, const struct FVector& LocalSpaceImpactNormal, const bool bWasTargetingWhenProjectileFired, UObject* HitEntity)
{
    auto Owner = Weapon->GetPlayerController();
    auto Component = Hit.Component.Get();
    if (!Component) return;

    auto Actor = Component->GetOwner();
    if (!Actor) return;

    auto WeaponStats = Weapon->WeaponData->GetStats();
    if (!WeaponStats) return;

  //  if (Actor->IsActorBeingDestroyed()) return; // ig that can help with the crash
    //if (Actor->bCanBeDamaged != 1) return; // random crash here

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

        if (Actor->IsA(AFortPlayerPawn::StaticClass()))
        {
            if (!UFortKismetLibrary::OnSameSquad((AFortPlayerPawnAthena*)Owner->MyFortPawn, (AFortPlayerPawnAthena*)Actor))
            {
                Owner->MyFortPawn->NetMulticast_Athena_BatchedDamageCues(SharedCue, NonSharedCue);
            }   
        }
    };

    if (bUseExperimentalDamaging)
    {
        auto a =  (UFortMovementComp_Character*)Owner->MyFortPawn->CharacterMovement;
        SendCueNew(WeaponStats->EnvDmgPB);
        UGameplayStatics::ApplyPointDamage(Actor, WeaponStats->EnvDmgPB, Hit.Location, Hit, Owner, Weapon, a->ImpactDamageTypeClass);
        Actor->ForceNetUpdate();
    }
}

void FortWeaponRanged::ServerNotifyPawnHit(UObject* Context, FFrame& Stack)
{
    float ProjectileStartTimestamp;
    bool bWasTargetingWhenProjectileFired;
    UObject* HitEntity;
    FHitResult& Hit = Stack.StepCompiledInRef<FHitResult>();
    FVector& ProjectileOriginPosition = Stack.StepCompiledInRef<FVector>();
    Stack.StepCompiledIn(&ProjectileStartTimestamp);
    TArray<uint8>& Context1 = Stack.StepCompiledInRef<TArray<uint8>>();
    FVector& LocalSpaceImpactPoint = Stack.StepCompiledInRef<FVector>();
    FVector& LocalSpaceImpactNormal = Stack.StepCompiledInRef<FVector>();
    Stack.StepCompiledIn(&bWasTargetingWhenProjectileFired);
    Stack.StepCompiledIn(&HitEntity);
    ServerNotifyPawnHit_Implemenation((AFortWeaponRanged*)Context, Hit, ProjectileOriginPosition, ProjectileStartTimestamp, Context1, LocalSpaceImpactNormal, LocalSpaceImpactNormal, bWasTargetingWhenProjectileFired, HitEntity);
}

void FortWeaponRanged::Init() {
    UHook* Hook = new UHook();

    Hook->Detour = ServerNotifyPawnHit;
    Hook->Path = "/Script/FortniteGame.FortWeaponRanged.ServerNotifyPawnHit";
    UKismetHookingLibrary::Hook(Hook, Exec);

    delete Hook;
}