#pragma once
#include "pch.h"

namespace FortPickup {
	DefineOriginal(void, GivePickupTo, AFortPickup* Pickup, IFortInventoryOwnerInterface* InventoryOwner, bool DestoryAfterPickup, double _XMM3_8);
	void Init();
}
inline UFortWeaponModSetData* AFortPickup::GetWeaponModsetData() {
	for (auto State : this->PrimaryPickupItemEntry.StateValuesConstObject) {
		if (State.StateType.TagName == UKismetStringLibrary::Conv_StringToName(L"ItemStateValue.ConstObject.ModSet")) {
			return (UFortWeaponModSetData*)State.ObjectData.Get();
		}
	}
	return nullptr;
}