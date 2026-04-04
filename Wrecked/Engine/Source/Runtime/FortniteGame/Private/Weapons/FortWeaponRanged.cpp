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
        bool bIsPlayer = Actor->IsA(AFortPlayerPawn::StaticClass());
        auto Pawn = (AFortPlayerPawn*)Actor;
        
        if (bIsPlayer && EFortDamageZone::Critical == Pawn->GetDamageZone(Hit))
            return WeaponStats->DmgPB * WeaponStats->DamageZone_Critical;
    
        return bIsPlayer ? WeaponStats->DmgPB * WeaponStats->DamageZone_Normal :  WeaponStats->EnvDmgPB * WeaponStats->DamageZone_Normal;
    };
    
    if (bUseExperimentalDamaging)
    {
        auto Damage = CalculateDamage(Hit, WeaponStats);
        auto MovementComp =  (UFortMovementComp_Character*)Owner->MyFortPawn->CharacterMovement;
        auto FinalDamage = Damage / 2;
     //   std::cout << "FinalDamage:" << Damage << std::endl;
        SendCue(Damage);
        if (!Actor->IsA(AFortPlayerPawn::StaticClass()))
        {
            UGameplayStatics::ApplyPointDamage(Actor, FinalDamage, ProjectileOriginPosition, Hit, Owner, Weapon, MovementComp->ImpactDamageTypeClass);
           // UGameplayStatics::ApplyDamage(Actor, FinalDamage, Owner, Weapon, MovementComp->ImpactDamageTypeClass);
        }
        if (Actor->IsA(AFortPlayerPawn::StaticClass()))
        {
            auto TargetPawn = (AFortPlayerPawnAthena*)Actor;
            float Shield = TargetPawn->GetShield();

            if (Shield > 0) {
                if (Shield < FinalDamage) {
                    float Remain = FinalDamage - Shield;
                    TargetPawn->SetShield(0);

                    UGameplayStatics::ApplyPointDamage(TargetPawn, Remain, ProjectileOriginPosition, Hit, Owner, Weapon, MovementComp->ImpactDamageTypeClass);
                    TargetPawn->SpawnCameraLensEffectDirectional(TargetPawn->ScreenEffectShieldDamage, Weapon, Hit, true);
                } else {
                    TargetPawn->SetShield(Shield - FinalDamage);
                    TargetPawn->SpawnCameraLensEffectDirectional(TargetPawn->ScreenEffectShieldDamage, Weapon, Hit, true);
                    // thats scuffed but whatever
                    UGameplayStatics::ApplyPointDamage(TargetPawn, 0.001f, ProjectileOriginPosition, Hit, Owner, Weapon, MovementComp->ImpactDamageTypeClass);
                }
            } else {
                UGameplayStatics::ApplyPointDamage(TargetPawn, FinalDamage, ProjectileOriginPosition, Hit, Owner, Weapon, MovementComp->ImpactDamageTypeClass);
                TargetPawn->SpawnCameraLensEffectDirectional(TargetPawn->ScreenEffectHealthDamage, Weapon, Hit, true);
            }
        }
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