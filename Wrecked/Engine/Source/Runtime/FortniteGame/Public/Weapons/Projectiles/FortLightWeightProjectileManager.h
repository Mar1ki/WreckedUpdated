#pragma once
#include "pch.h"

namespace FortLightWeightProjectileManager {
	DefineOriginal(void, ProjectileRequest, AFortLightweightProjectileManager* ProjectileManager, FWeakObjectPtr PawnObject, FWeakObjectPtr WeaponDef, UClass** ProjectileConfigPTR, FVector BuletStartPosition, FVector Direction, ELightweightProjectileRequestType Type, int a8, unsigned int a9);

	void Init();
}