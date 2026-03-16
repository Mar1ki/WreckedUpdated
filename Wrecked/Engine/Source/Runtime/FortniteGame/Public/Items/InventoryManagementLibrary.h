#pragma once
#include "pch.h"

#include "Engine/Source/Runtime/CoreUObject/Public/UObject/Stack.h"

namespace InventoryManagementLibrary {

	bool RemoveItem(UObject* Object, FFrame& Params, bool* Return);
	void RemoveItems(UObject* Object, FFrame& Params);
	void Init();
}
