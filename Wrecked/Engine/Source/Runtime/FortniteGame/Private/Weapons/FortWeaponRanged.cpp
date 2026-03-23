#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Weapons/FortWeaponRanged.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"

void ServerNotifyPawnHit_Implemenation(AFortWeaponRanged* Weapon, const struct FHitResult& Hit, const struct FVector& ProjectileOriginPosition, float ProjectileStartTimestamp, const TArray<uint8>& Context, const struct FVector& LocalSpaceImpactPoint, const struct FVector& LocalSpaceImpactNormal, const bool bWasTargetingWhenProjectileFired, UObject* HitEntity) {

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