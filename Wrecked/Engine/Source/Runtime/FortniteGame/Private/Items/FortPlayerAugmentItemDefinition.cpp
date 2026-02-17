#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Items/FortPlayerAugmentItemDefinition.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"

void FortPlayerAugmentItemDefinition::OnItemInstanceAdded(UFortPlayerAugmentItemDefinition* Item, IFortInventoryOwnerInterface* InventoryOwner, UFortItem* ItemInstance, bool bLoadedFromRecord)
{
	std::cout << "Dummy" << std::endl;

	OnItemInstanceAddedOG(Item, InventoryOwner, ItemInstance, bLoadedFromRecord);
}
void FortPlayerAugmentItemDefinition::OnItemInstanceRemoved(UFortPlayerAugmentItemDefinition* Item, IFortInventoryOwnerInterface* InventoryOwner, UFortItem* ItemInstance, bool bLoadedFromRecord)
{
	std::cout << "Dummy2"  << std::endl;
}

void FortPlayerAugmentItemDefinition::Init() {

    UHook* Hook = new UHook();
	
    Hook->Address = 0x428 / 8;
    Hook->Class = UFortPlayerAugmentItemDefinition::StaticClass();
    Hook->Detour = OnItemInstanceAdded;
	Hook->Original = (void**)&OnItemInstanceAddedOG;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);

	
    Hook->Address = 0x430 / 8;
    Hook->Class = UFortPlayerAugmentItemDefinition::StaticClass();
    Hook->Detour = OnItemInstanceRemoved;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);
	
    delete Hook;
}