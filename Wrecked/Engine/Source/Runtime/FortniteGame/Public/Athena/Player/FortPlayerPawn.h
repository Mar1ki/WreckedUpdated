#pragma once
#include "pch.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

namespace FortPlayerPawn {
	DefineOriginal(void, ServerHandlePickupInfo, AFortPlayerPawn* Pawn, AFortPickup* Pickup, FFortPickupRequestInfo& Params);
	DefineOriginal(__int64*, OnAboutToEnterBackpack, AFortPickup* Pickup);
	static void ServerSendZiplineState(UObject* Context, FFrame& Params);

	void Init();
}