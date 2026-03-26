#pragma once
#include "pch.h"

namespace FortPickup {
	DefineOriginal(void, GivePickupTo, AFortPickup* Pickup, IFortInventoryOwnerInterface* InventoryOwner, bool DestoryAfterPickup, double _XMM3_8);
	void Init();
}