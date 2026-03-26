#pragma once
#include "pch.h"

namespace FortInventoryOwnerInterface {
	bool RemoveInventoryItem(IFortInventoryOwnerInterface* Interface, FGuid* ItemGuid, int Count);
	bool __fastcall CreateInventory(IFortInventoryOwnerInterface* Interface);
	void __fastcall InitializeInventory(IFortInventoryOwnerInterface* Interface, const TArray<FItemAndCount> ItemList);
	AFortQuickBars* __fastcall GetQuickBars(IFortInventoryOwnerInterface* Interface);
	bool HasInfiniteAmmo(IFortInventoryOwnerInterface* Interface, const UFortItemDefinition* Item);
	bool HasInfiniteConsumables(IFortInventoryOwnerInterface* Interface);

	void Init();
}