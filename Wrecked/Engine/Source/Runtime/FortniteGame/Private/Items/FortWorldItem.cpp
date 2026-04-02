#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Items/FortWorldItem.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"

bool FortWorldItem::SetPhantomReserveAmmo(UFortWorldItem* Item, int InCount)
{
	std::cout << "phantom" << std::endl;
	if (!Item)
		return false;

	auto Inventory = Item->GetOwningController()->GetInventory();

	if (!Inventory)
		return false;

	auto Entry = Inventory->GetItemEntryByGuid(Item->GetItemGuid());

	if (!Entry)
		return false;

	Entry->PhantomReserveAmmo = InCount;
	Inventory->ReplaceEntry(*Entry);
	Inventory->Update(Entry);
	return true;
}

bool FortWorldItem::SetLoadedAmmo(UFortWorldItem* Item, int InCount)
{
	auto Inventory = Item->GetOwningController()->GetInventory();

	auto Stats = ((UFortWeaponItemDefinition*)Item->ItemEntry.ItemDefinition)->GetStats();
	if (InCount > Stats->ClipSize)
		return false;
	Item->ItemEntry.LoadedAmmo = InCount;
	Inventory->Update(&Item->ItemEntry);
	
	auto Entry = Inventory->GetItemEntryByGuid(Item->GetItemGuid());

	if (!Entry)
		return false;

	Entry->LoadedAmmo = InCount;
	Inventory->Update(Entry);
	return true;
}
void FortWorldItem::Init() {

	UHook* Hook = new UHook();

	Hook->Address = 0x600 / 8;
	Hook->Detour = SetLoadedAmmo;
	Hook->Class = UFortWorldItem::StaticClass();
	UKismetHookingLibrary::Hook(Hook, EveryVFT);

	Hook->Address = 0x608 / 8;
	Hook->Detour = SetPhantomReserveAmmo;
	Hook->Class = UFortWorldItem::StaticClass();
	UKismetHookingLibrary::Hook(Hook, EveryVFT);

	delete Hook;
}