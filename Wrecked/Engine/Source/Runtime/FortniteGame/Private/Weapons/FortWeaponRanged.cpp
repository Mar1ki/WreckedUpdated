#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Weapons/FortWeaponRanged.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
void ServerNotifyPawnHit_Implemenation(AFortWeaponRanged* Weapon, const FHitResult& Hit, const FVector& ProjectileOriginPosition, float ProjectileStartTimestamp, const TArray<uint8>& Context, const FVector& LocalSpaceImpactPoint, const FVector& LocalSpaceImpactNormal, const bool bWasAimingDownSights, UObject* HitEntity)
{
    auto Owner = Weapon->GetPlayerController();
    auto Component = Hit.Component.Get();
    if (!Component) return;

    auto Actor = Component->GetOwner();
    if (!Actor) return;

    auto WeaponStats = Weapon->WeaponData->GetStats();
    if (!WeaponStats) return;
    
    // the main thing is we dont calculate the damage here okay 
    auto SendCue = [&](float Damage) {
        bool bIsPlayer = Actor->IsA(AFortPlayerPawn::StaticClass());

        if (bIsPlayer && UFortKismetLibrary::OnSameSquad((AFortPlayerPawnAthena*)Owner->MyFortPawn, (AFortPlayerPawnAthena*)Actor))return;
        
        FAthenaBatchedDamageGameplayCues_Shared SharedCue{};
        SharedCue.Location = (FVector_NetQuantize10)Hit.ImpactPoint;
        SharedCue.Normal = Hit.ImpactNormal;
        SharedCue.LocalSpaceImpactPoint = (FVector_NetQuantize10)LocalSpaceImpactPoint;
        SharedCue.LocalSpaceImpactNormal = (FVector_NetQuantizeNormal)LocalSpaceImpactNormal;
        SharedCue.Magnitude = Damage;
        SharedCue.bWeaponActivate = true;
        SharedCue.bIsBallistic = true;
        SharedCue.bIsValid = true;
        if (bIsPlayer)
        {
            auto Pawn = (AFortPlayerPawn*)Actor;
            auto Shield = Pawn->GetShield();
            SharedCue.bIsCritical = Hit.BoneName.ToString() == "Head";
            SharedCue.bIsShield = Shield > 0;
            SharedCue.bIsShieldDestroyed = Shield > 0 && Shield <= Damage;
        }
        // this is prob the best way u can do it
        FAthenaBatchedDamageGameplayCues_NonShared NonSharedCue{};
        NonSharedCue.HitActor = bIsPlayer ? Actor : nullptr;
        NonSharedCue.NonPlayerHitActor = bIsPlayer ? nullptr : Actor;
        
        Owner->MyFortPawn->NetMulticast_Athena_BatchedDamageCues(SharedCue, NonSharedCue);
    };
    // it is alr but i think we need to make it a bit better ngl
    // we could prob check if its a player if not we will use the env dmg 
    auto CalculateDamage = [&](const FHitResult& Hit, const FFortRangedWeaponStats* WeaponStats) -> float
    {
        auto BoneName = Hit.BoneName.ToString();
    
        if (BoneName == "Head")
            return WeaponStats->DmgPB * WeaponStats->DamageZone_Critical;
    
        return WeaponStats->DmgPB * WeaponStats->DamageZone_Normal;
    };
    
    if (bUseExperimentalDamaging)
    {
        auto FinalDamage = CalculateDamage(Hit, WeaponStats);
        std::cout << "FinalDamage:" << FinalDamage << std::endl;
        auto MovementComp =  (UFortMovementComp_Character*)Owner->MyFortPawn->CharacterMovement;
        SendCue(FinalDamage);
        UGameplayStatics::ApplyPointDamage(Actor, FinalDamage, ProjectileOriginPosition, Hit, Owner, Weapon, MovementComp->ImpactDamageTypeClass);
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
    ServerNotifyPawnHit_Implemenation((AFortWeaponRanged*)Context, Hit, ProjectileOriginPosition, ProjectileStartTimestamp, Context1, LocalSpaceImpactPoint, LocalSpaceImpactNormal, bWasTargetingWhenProjectileFired, HitEntity);
}

void FortWeaponRanged::Init() {
    UHook* Hook = new UHook();

    Hook->Detour = ServerNotifyPawnHit;
    Hook->Path = "/Script/FortniteGame.FortWeaponRanged.ServerNotifyPawnHit";
    UKismetHookingLibrary::Hook(Hook, Exec);

    delete Hook;
}