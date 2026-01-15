#pragma once
#include "pch.h"

namespace BuildingContainer
{
	DefineOriginal(bool, SpawnLoot, ABuildingContainer* Container);
	DefineOriginal(void, OnDamageServer, ABuildingSMActor* BuildingActor, float Damage, FGameplayTagContainer& DamageTags, FVector& Momentum, FHitResult& HitInfo, AFortPlayerControllerAthena* InstigatedBy, AActor* DamageCauser, FGameplayEffectContextHandle& EffectContext);
	DefineOriginal(void, OnDied_Event, UObject* Context, FFrame& Params)
	void Setup();
}