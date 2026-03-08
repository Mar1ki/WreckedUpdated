#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Items/FortPlayerAugmentItemDefinition.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"

void FortPlayerAugmentItemDefinition::OnItemInstanceAdded(UFortPlayerAugmentItemDefinition* Item, IFortInventoryOwnerInterface* InventoryOwner, UFortItem* ItemInstance, bool bLoadedFromRecord)
{
	void* retAddr = _ReturnAddress();
	std::cout << "Added called from: 0x" << std::hex << ((uintptr_t)retAddr - ImageBase) << std::endl;

	auto Controller = ItemInstance->GetOwningController();
	auto Pawn = Controller->MyFortPawn;
	auto Interface = Pawn->PlayerState->GetInterfaceAddress<IAbilitySystemInterface>();
	TScriptInterface<IAbilitySystemInterface> ScriptInterface;
	ScriptInterface.ObjectPointer = Pawn->AbilitySystemComponent->GetOwner();
	ScriptInterface.InterfacePointer = Interface;


	auto EffectHandle = FGameplayEffectContextHandle();
	Pawn->AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(Item->GameplayEffect.Get(), 0, EffectHandle);
	UFortKismetLibrary::EquipFortAbilitySet(ScriptInterface, Item->AbilitySet.Get(), nullptr);
	
	OnItemInstanceAddedOG(Item, InventoryOwner, ItemInstance, bLoadedFromRecord);
}
void FortPlayerAugmentItemDefinition::OnItemInstanceRemoved(UFortPlayerAugmentItemDefinition* Item, IFortInventoryOwnerInterface* InventoryOwner, const int ok)
{
	auto retAddr = (uintptr_t)_ReturnAddress() - ImageBase;
	if (retAddr == 0x9B8CC57) return; // thats so ahh

	
	std::cout << "hello" << std::endl;
	
}

void FortPlayerAugmentItemDefinition::Init() {

    UHook* Hook = new UHook();
	
	Hook->Address = 133;
    Hook->Class = UFortPlayerAugmentItemDefinition::StaticClass();
    Hook->Detour = OnItemInstanceAdded;
	Hook->Original = (void**)&OnItemInstanceAddedOG;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);

	
	Hook->Address = 134;
    Hook->Class = UFortPlayerAugmentItemDefinition::StaticClass();
    Hook->Detour = OnItemInstanceRemoved;
    UKismetHookingLibrary::Hook(Hook, EveryVFT);
	
    delete Hook;
}