#pragma once
#include "pch.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

namespace FortPlayerPawn {
	DefineOriginal(__int64*, OnAboutToEnterBackpack, AFortPickup* Pickup);
	static void ServerSendZiplineState(UObject* Context, FFrame& Params);
	DefineOriginal(void, TryAutoPickup, AFortPlayerPawn* Pawn, AFortPickupAthena* Pickup);
	DefineOriginal(void, test, __int64* a1, __int64* a2);

	void Init();
}

inline void AFortPlayerPawn::HandleZiplineStateChanged()
{
	void (*HandleZiplineStateChangedInternal)(AFortPlayerPawn*) = reinterpret_cast<void(*)(AFortPlayerPawn*)>(ImageBase + 0xA12F5D8);

	HandleZiplineStateChangedInternal(this);
}


inline void AFortPickup::SetPickupTarget(AFortPlayerPawn* PlayerPawn, float FlyTime, FVector Direction)
{
	void (*SetPickupTargetInternal)(AFortPickup*, AFortPlayerPawn* PlayerPawn, float FlyTime, FVector Direction) = reinterpret_cast<void(*)(AFortPickup*, AFortPlayerPawn* PlayerPawn, float FlyTime, FVector Direction)>(ImageBase + 0x9BB8768);

	SetPickupTargetInternal(this, PlayerPawn, FlyTime, Direction);
}

