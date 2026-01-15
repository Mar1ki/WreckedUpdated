#include "pch.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"

FFortRangedWeaponStats* UFortWeaponItemDefinition::GetStats()
{
    if (!WeaponStatHandle.DataTable)
        return nullptr;

    auto Stats = WeaponStatHandle.DataTable->Search([this](FName &Key, uint8_t *Value)
                                                           { return this->WeaponStatHandle.RowName == Key && Value; });

    return Stats ? *(FFortRangedWeaponStats **)Stats : nullptr;
}

void AFortInventory::Update(FFortItemEntry *Entry)
{
    this->bRequiresLocalUpdate = true;
    this->HandleInventoryLocalUpdate();

    return Entry ? this->Inventory.MarkItemDirty(*Entry) : this->Inventory.MarkArrayDirty();
}
UFortWorldItem** AFortInventory::GetWorldItemByGuid(FGuid Guid) { return Inventory.ItemInstances.Search([Guid](class UFortWorldItem*& WorldItem) {return WorldItem->ItemEntry.ItemGuid == Guid; }); }
UFortWorldItem** AFortInventory::GetWorldItemByDef(UFortItemDefinition* Def) { return Inventory.ItemInstances.Search([Def](class UFortWorldItem*& WorldItem) {return WorldItem->ItemEntry.ItemDefinition == Def; }); }

UFortWeaponModSetData* FFortItemEntry::GetWeaponModsetData() {
    for (auto State : this->StateValuesConstObject) {
        if (State.StateType.TagName == UKismetStringLibrary::Conv_StringToName(L"ItemStateValue.ConstObject.ModSet")) {
            return (UFortWeaponModSetData*)State.ObjectData.Get();
        }
    }
    return nullptr;
}

FFortItemEntry* AFortInventory::AddItem(UFortItemDefinition* Def, int32 Count, int LoadedAmmo, int32 Level, UFortWeaponModSetData* Modset)
{
    UFortWorldItem *Item = (UFortWorldItem *)Def->CreateTemporaryItemInstanceBP(Count, Level);
    if (!Item) return nullptr;
    auto Controller = Cast<AFortPlayerController>(GetOwner());
    
    Item->ItemEntry.LoadedAmmo = LoadedAmmo;
    Item->SetOwningControllerForTemporaryItem(Controller);
    
    if (auto Weapon = Cast<UFortWeaponRangedItemDefinition>(Def))
    {
        auto Stats = Weapon->GetStats();
    	if (Weapon->bUsesPhantomReserveAmmo)
    	{
    		Item->ItemEntry.PhantomReserveAmmo = (Stats->InitialClips - 1) * Stats->ClipSize;
    	}
    }
// theres our issue when reloading gng prob
    if (Def->IsA(UFortAmmoItemDefinition::StaticClass()) || Def->IsA(UFortResourceItemDefinition::StaticClass()))
    {
        FFortItemEntryStateValue Value{};
        Value.IntValue = true;
        Value.StateType = EFortItemEntryState::ShouldShowItemToast;
        Item->ItemEntry.StateValues.Add(Value);
    }
    if (auto Weapon = Cast<UFortWeaponRangedItemDefinition>(Def) && Modset) {
        ApplyWeaponModset(&Item->ItemEntry, Modset);
        FFortItemEntryStateValueConstObject NewState;
        NewState.ObjectData = Modset;
        FGameplayTag ModsetTag = *(FGameplayTag*)(ImageBase + 0x12BCBA54);
        NewState.StateType = ModsetTag;
        Item->ItemEntry.StateValuesConstObject.Add(NewState);
    }
    
    Inventory.ReplicatedEntries.Add(Item->ItemEntry);
    Inventory.ItemInstances.Add(Item);
    
    Update(&Item->ItemEntry);

    return &Item->ItemEntry;
}

bool AFortInventory::ReplaceEntry(FFortItemEntry& Entry)
{
    bool bSuccess = false;
    if (!this) 
        return bSuccess;

    if (Entry.Count <= 0)
    {
        Remove(Entry.ItemGuid);
        return bSuccess = true;
    }
    
    for (int i = 0; i < Inventory.ItemInstances.Num(); i++)
    {
        if (Inventory.ItemInstances[i]->ItemEntry.ItemGuid == Entry.ItemGuid)
        {
            Inventory.ItemInstances[i]->ItemEntry = Entry;
            
            bSuccess = true;
            break;
        }
    }

    for (int i = 0; i < Inventory.ReplicatedEntries.Num(); i++)
    {
        if (Inventory.ReplicatedEntries[i].ItemGuid == Entry.ItemGuid)
        {
            Inventory.ReplicatedEntries[i] = Entry;
            Inventory.MarkItemDirty(Entry);
            bSuccess = true;
            break;
        }
    }

    if (bSuccess)
        Update(&Entry);
    
    return bSuccess;
}

void AFortInventory::Remove(FGuid ItemGuid)
{
    bool bSuccess = false;
    if (!this) 
        return;
    
    for (int i = 0; i < Inventory.ItemInstances.Num(); i++)
    {
        if (Inventory.ItemInstances[i]->ItemEntry.ItemGuid == ItemGuid)
        {
            Inventory.ItemInstances.Remove(i);
            bSuccess = true;
            break;
        }
    }

    for (int i = 0; i < Inventory.ReplicatedEntries.Num(); i++)
    {
        if (Inventory.ReplicatedEntries[i].ItemGuid == ItemGuid)
        {
            Inventory.ReplicatedEntries.Remove(i);
            bSuccess = true;
            break;
        }
    }

    if (bSuccess)
        Update(nullptr);
}


void AFortInventory::UpdateLoadedAmmo(AFortPlayerController* PC, AFortWeapon* Weapon)
{
	auto Inventory = (AFortInventory*)PC->WorldInventory.ObjectPointer;
	
	if (!Inventory || !Inventory->Inventory.ReplicatedEntries.IsValid())
		return;
		
	for (size_t i = 0; i < Inventory->Inventory.ReplicatedEntries.Num(); i++)
	{
		if (!Inventory->Inventory.ReplicatedEntries.IsValidIndex(i))
			continue;
			
		if (Inventory->Inventory.ReplicatedEntries[i].ItemGuid == Weapon->ItemEntryGuid)
		{
			Inventory->Inventory.ReplicatedEntries[i].LoadedAmmo = Weapon->AmmoCount;
			
			Inventory->bRequiresLocalUpdate = true;
			Inventory->HandleInventoryLocalUpdate();
			Inventory->Inventory.MarkItemDirty(Inventory->Inventory.ReplicatedEntries[i]);
			
			break;
		}
	}
}

bool AFortInventory::RemoveItem(AFortPlayerController* PC, UFortWorldItemDefinition* ItemDef, int Count)
{
	auto Inventory = (AFortInventory*)PC->WorldInventory.ObjectPointer;
	
	if (!Inventory || !ItemDef || Count <= 0)
		return false;
		
	if (!Inventory->Inventory.ReplicatedEntries.IsValid())
		return false;
	
	int RemainingToRemove = Count;
	
	for (int i = Inventory->Inventory.ReplicatedEntries.Num() - 1; i >= 0; i--)
	{
		if (!Inventory->Inventory.ReplicatedEntries.IsValidIndex(i))
			continue;
			
		auto& Entry = Inventory->Inventory.ReplicatedEntries[i];
		
		if (Entry.ItemDefinition == ItemDef)
		{
			if (Entry.Count <= RemainingToRemove)
			{
				RemainingToRemove -= Entry.Count;
				for (int j = 0; j < Inventory->Inventory.ItemInstances.Num(); j++)
				{
					if (Inventory->Inventory.ItemInstances[j]->ItemEntry.ItemGuid == Entry.ItemGuid)
					{
						Inventory->Inventory.ItemInstances.Remove(j);
						break;
					}
				}
				
				Inventory->Inventory.ReplicatedEntries.Remove(i);
				
				if (RemainingToRemove <= 0)
					break;
			}
			else
			{
				Entry.Count -= RemainingToRemove;
				
				for (int j = 0; j < Inventory->Inventory.ItemInstances.Num(); j++)
				{
					if (Inventory->Inventory.ItemInstances[j]->ItemEntry.ItemGuid == Entry.ItemGuid)
					{
						Inventory->Inventory.ItemInstances[j]->ItemEntry.Count = Entry.Count;
						break;
					}
				}
				
				Inventory->Inventory.MarkItemDirty(Entry);
				RemainingToRemove = 0;
				break;
			}
		}
	}
	if (RemainingToRemove < Count)
	{
		Inventory->bRequiresLocalUpdate = true;
		Inventory->HandleInventoryLocalUpdate();
		Inventory->Inventory.MarkArrayDirty();
		return true;
	}
	return false;
}