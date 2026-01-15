#pragma once
#include "pch.h"

namespace FortInventoryOwnerInterface {
	bool RemoveInventoryItem(IFortInventoryOwnerInterface* Interface, FGuid* ItemGuid, int Count);
	bool __fastcall CreateInventory(IFortInventoryOwnerInterface* Interface);
	void __fastcall InitializeInventory(IFortInventoryOwnerInterface* Interface, const TArray<FItemAndCount> ItemList);
	void Init();
}