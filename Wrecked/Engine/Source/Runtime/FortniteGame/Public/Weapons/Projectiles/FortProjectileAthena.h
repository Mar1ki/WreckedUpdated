#pragma once
#include "pch.h"

namespace FortProjectileAthena {
	DefineOriginal(void, ServerNotifyPawnHit, AFortProjectileAthena* projectile, FHitResult& Hit, const TArray<uint8>& Context);

	void Init();
}