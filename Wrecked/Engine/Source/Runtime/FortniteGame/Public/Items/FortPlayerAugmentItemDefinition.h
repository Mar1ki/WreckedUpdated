#pragma once
#include "pch.h"


namespace FortPlayerAugmentItemDefinition
{

    DefineOriginal(void, OnItemInstanceAdded, UFortPlayerAugmentItemDefinition* Item,IFortInventoryOwnerInterface* InventoryOwner, UFortItem* Item2, bool bLoadedFromRecord);
void OnItemInstanceRemoved(UFortPlayerAugmentItemDefinition* Item, IFortInventoryOwnerInterface* InventoryOwner, const int ok);
    void Init();
}
