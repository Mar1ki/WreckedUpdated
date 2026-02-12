#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Kismet/FortKismetLibrary.h"

#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Weapons/FortWeaponModFunctionLibrary.h"

std::vector<UDataTable*> LootTierDatas;
std::vector<UDataTable*> LootPackageDatas;

int GetItemLevel(const FDataTableCategoryHandle& LootLevelData, int WorldLevel)
{
	auto DataTable = LootLevelData.DataTable;

	if (!DataTable)
		return 0;

	if (!LootLevelData.ColumnName.ComparisonIndex)
		return 0;

	if (!LootLevelData.RowContents.ComparisonIndex)
		return 0;

	std::vector<FFortLootLevelData*> OurLootLevelDatas;

	for (auto& LootLevelDataPair : LootLevelData.DataTable->RowMap)
	{
		if (((FFortLootLevelData*)(LootLevelDataPair.Second))->category != LootLevelData.RowContents)
			continue;

		OurLootLevelDatas.push_back(((FFortLootLevelData*)(LootLevelDataPair.Second)));
	}

	if (OurLootLevelDatas.size() > 0)
	{
		int PickedIndex = -1;
		int PickedLootLevel = 0;

		for (int i = 0; i < OurLootLevelDatas.size(); i++)
		{
			auto CurrentLootLevelData = OurLootLevelDatas.at(i);

			if (CurrentLootLevelData->LootLevel <= WorldLevel && CurrentLootLevelData->LootLevel > PickedLootLevel)
			{
				PickedLootLevel = CurrentLootLevelData->LootLevel;
				PickedIndex = i;
			}
		}

		if (PickedIndex != -1)
		{
			auto PickedLootLevelData = OurLootLevelDatas.at(PickedIndex);

			const auto PickedMinItemLevel = PickedLootLevelData->MinItemLevel;
			const auto PickedMaxItemLevel = PickedLootLevelData->MaxItemLevel;
			auto v15 = PickedMaxItemLevel - PickedMinItemLevel;

			if (v15 + 1 <= 0)
			{
				v15 = 0;
			}
			else
			{
				auto v16 = (int)(float)((float)((float)rand() * 0.000030518509) * (float)(v15 + 1));
				if (v16 <= v15)
					v15 = v16;
			}

			return v15 + PickedMinItemLevel;
		}
	}

	return 0;
}

int PickLevel(UFortWorldItemDefinition* def, int PreferredLevel)
{

	const int MinLevel = def->MinLevel;
	const int MaxLevel = def->MaxLevel;

	int PickedLevel = 0;

	if (PreferredLevel >= MinLevel)
		PickedLevel = PreferredLevel;

	if (MaxLevel >= 0)
	{
		if (PickedLevel <= MaxLevel)
			return PickedLevel;
		return MaxLevel;
	}

	return PickedLevel;
}

float GetAmountOfLootPackagesToDrop(FFortLootTierData* LootTierData, int OriginalNumberLootDrops)
{
	if (LootTierData->LootPackageCategoryMinArray.Num() != LootTierData->LootPackageCategoryWeightArray.Num() || LootTierData->LootPackageCategoryMinArray.Num() != LootTierData->LootPackageCategoryWeightArray.Num())
		return 0;

	float MinimumLootDrops = 0;

	if (LootTierData->LootPackageCategoryMinArray.Num() > 0)
	{
		for (int i = 0; i < LootTierData->LootPackageCategoryMinArray.Num(); ++i)
		{
			MinimumLootDrops += LootTierData->LootPackageCategoryMinArray[i];
		}
	}

	int SumLootPackageCategoryWeightArray = 0;

	if (LootTierData->LootPackageCategoryWeightArray.Num() > 0)
	{
		for (int i = 0; i < LootTierData->LootPackageCategoryWeightArray.Num(); ++i)
		{
			if (LootTierData->LootPackageCategoryWeightArray[i] > 0)
			{
				auto LootPackageCategoryMaxArrayIt = LootTierData->LootPackageCategoryMaxArray[i];

				float IDK = 0;

				if (LootPackageCategoryMaxArrayIt < 0 || IDK < LootPackageCategoryMaxArrayIt)
				{
					SumLootPackageCategoryWeightArray += LootTierData->LootPackageCategoryWeightArray[i];
				}
			}
		}
	}

	while (SumLootPackageCategoryWeightArray > 0)
	{
		float v29 = (float)rand() * 0.000030518509f;

		float v35 = (int)(float)((float)((float)((float)SumLootPackageCategoryWeightArray * v29)
			+ (float)((float)SumLootPackageCategoryWeightArray * v29))
			+ 0.5f) >> 1;

		MinimumLootDrops++;

		if (MinimumLootDrops >= OriginalNumberLootDrops)
			return MinimumLootDrops;

		SumLootPackageCategoryWeightArray--;
	}

	return MinimumLootDrops;
}

void AddLootTierData(UDataTable* DataTable)
{
	if (!DataTable || IsBadReadPtr(DataTable, 8) || !DataTable->Class)
		return;
	if (auto CompositeDataTable = Cast<UCompositeDataTable>(DataTable)) {
		for (auto Table : CompositeDataTable->ParentTables) {
			LootTierDatas.push_back(Table);
		}
	}
	else {
		LootTierDatas.push_back(DataTable);
	}
}

void AddLootPackageData(UDataTable* DataTable)
{
	if (!DataTable || IsBadReadPtr(DataTable, 8) || !DataTable->Class)
		return;
	if (auto CompositeDataTable = Cast<UCompositeDataTable>(DataTable)) {
		for (auto Table : CompositeDataTable->ParentTables) {
			LootPackageDatas.push_back(Table);
		}
	}
	else {
		LootPackageDatas.push_back(DataTable);
	}
}

void SetupLTDS()
{
	auto GameMode = GetGameMode();
	auto GameState = GetGameState();
	if (!GameMode || !GameState)
		return;

	auto Playlist = GameState->CurrentPlaylistInfo.BasePlaylist;
	if (!Playlist)
		return;

	AddLootTierData(Playlist->LootTierData.Get());
	AddLootPackageData(Playlist->LootPackages.Get());

	auto GameFeatures = GetObjectsOfClass<UFortGameFeatureData>();

	for (UFortGameFeatureData* GameFeature : GameFeatures)
	{
		AddLootTierData(GameFeature->DefaultLootTableData.LootTierData.Get());
		AddLootPackageData(GameFeature->DefaultLootTableData.LootPackageData.Get());
		for (auto Tag : Playlist->GameplayTagContainer.GameplayTags)
		{
			auto OvverideLootTableDataIt = GameFeature->PlaylistOverrideLootTableData.Find(Tag, [](const FGameplayTag& Left, const FGameplayTag& Wright) {return Left == Wright; });

			if (OvverideLootTableDataIt.IsValid())
			{
				auto OvverideLootTableData = GameFeature->PlaylistOverrideLootTableData[OvverideLootTableDataIt.GetIndex()].Second;
				AddLootPackageData(OvverideLootTableData.LootPackageData.Get());
			}
		}
	}
}
template <typename T>
T* PickWeighted(std::vector<T*> Array) {
	float TotalWeight = std::accumulate(Array.begin(), Array.end(), 0.0f, [&](float acc, T*& p) { return acc + p->Weight; });
	float Number = WeightedRandom(TotalWeight);

	for (auto Element : Array)
	{
		float Weight = Element->Weight;
		if (Weight == 0)
			continue;

		if (Number <= Weight)
			return Element;

		Number -= Weight;
	}

	return nullptr;
}
bool ValidateLootPackage(FFortLootPackageData* LootPackage, FName LootPackageName, int LootPackageCategory, int WorldLevel) {
	if (LootPackage->LootPackageID != LootPackageName)
	{
		return false;
	}

	if (LootPackageCategory != -1 && LootPackage->LootPackageCategory != LootPackageCategory)
	{
		return false;
	}

	if (WorldLevel >= 0)
	{
		if (LootPackage->MaxWorldLevel >= 0 && WorldLevel > LootPackage->MaxWorldLevel)
			return 0;

		if (LootPackage->MinWorldLevel >= 0 && WorldLevel < LootPackage->MinWorldLevel)
			return 0;
	}

	return true;
}

FFortItemEntry FortKismetLibrary::ConstructItemEntry(UFortItemDefinition* ItemDefinition, int32 Count, int32 Level) {
	FFortItemEntry ItemEntry{};

	ItemEntry.MostRecentArrayReplicationKey = -1;
	ItemEntry.ReplicationID = -1;
	ItemEntry.ReplicationKey = -1;

	ItemEntry.ItemDefinition = ItemDefinition;
	ItemEntry.Count = Count;
	ItemEntry.Durability = 1.f;
	ItemEntry.GameplayAbilitySpecHandle = FGameplayAbilitySpecHandle(-1);
	ItemEntry.ParentInventory.ObjectIndex = -1;
	ItemEntry.Level = Level;

	if (auto Weapon = Cast<UFortWeaponItemDefinition>(ItemDefinition))
	{
		auto Stats = Weapon->GetStats();
		ItemEntry.LoadedAmmo = Stats->ClipSize;
		if (Weapon->bUsesPhantomReserveAmmo)
			ItemEntry.PhantomReserveAmmo = Stats->InitialClips * Stats->ClipSize;
	}

	return ItemEntry;
}

int FortKismetLibrary::GetMaxStackSize(UFortItemDefinition* Item) {
	static auto MaxStackSizeClass = (UClass*)SDK::UObject::FindObjectFast("ItemComponentData_MaxStackSize", EClassCastFlags::Struct);
	for (auto Struct : Item->DataList) {
		auto StructType = (UScriptStruct*)Struct.ScriptStruct.ObjectPtr.Handle.pointerOrRef;
		if (StructType) {
			if (StructType->IsSubclassOf(MaxStackSizeClass)) {
				auto MaxStackSizeStruct = (FItemComponentData_MaxStackSize*)Struct.Struct;
				if (MaxStackSizeStruct) {
					return MaxStackSizeStruct->MaxStackSize.Evaluate();
				}
			}
		}
	}
	return 1;
}

void PickLootFromLP(TArray<FFortItemEntry>& LootDrops, FName LootPackageName, int LootPackageCategory, int WorldLevel) {
	std::vector<FFortLootPackageData*> LootPackages;
	for (auto Data : LootPackageDatas)
	{
		if (!Data)
			continue;

		for (auto Pair : Data->RowMap)
		{
			auto LootPackageData = reinterpret_cast<FFortLootPackageData*>(Pair.Second);
			if (LootPackageData) {
				if (ValidateLootPackage(LootPackageData, LootPackageName, LootPackageCategory, WorldLevel))
					LootPackages.push_back(LootPackageData);

			}
		}
	}
	if (LootPackages.size() == 0)
		return;

	FName PickedPackageRowName;

	auto LootPackage = PickWeighted<FFortLootPackageData>(LootPackages);
	if (!LootPackage)
		return;

	if (LootPackage->LootPackageCall.Num() > 1)
	{
		for (auto i = 0; i < LootPackage->Count; i++)
			return PickLootFromLP(LootDrops, LootPackage->LootPackageCall.IsValid() ? UKismetStringLibrary::Conv_StringToName(LootPackage->LootPackageCall) : FName(0), 0, WorldLevel);
	}

	auto ItemDefinition = LootPackage->ItemDefinition.Get();;
	if (!ItemDefinition)
		return;
	auto WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemDefinition);
	int LoadedAmmo = WeaponItemDefinition ? WeaponItemDefinition->GetStats()->ClipSize : 0;

	auto WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemDefinition);

	if (!WorldItemDefinition)
		return;

	int ItemLevel = GetItemLevel(WorldItemDefinition->LootLevelData, WorldLevel);

	int Count = LootPackage->Count;

	if (Count > 0)
	{
		int FinalItemLevel = 0;

		if (ItemLevel >= 0)
			FinalItemLevel = ItemLevel;

		while (Count > 0)
		{
			int MaxStackSize = FortKismetLibrary::GetMaxStackSize(ItemDefinition);

			int CurrentCountForEntry = MaxStackSize;

			if (Count <= MaxStackSize)
				CurrentCountForEntry = Count;

			if (CurrentCountForEntry <= 0)
				CurrentCountForEntry = 0;

			auto ActualItemLevel = PickLevel(WorldItemDefinition, FinalItemLevel);
			LootDrops.Add(FortKismetLibrary::ConstructItemEntry(ItemDefinition, CurrentCountForEntry, ActualItemLevel));
			bool IsWeapon = LootPackage->LootPackageID.ToString().contains(".Weapon.") && WeaponItemDefinition;
			
			if (IsWeapon && WeaponItemDefinition->GetAmmoWorldItemDefinition_BP() && WeaponItemDefinition->GetAmmoWorldItemDefinition_BP() != WeaponItemDefinition)
			{
				LootDrops.Add(FortKismetLibrary::ConstructItemEntry(WeaponItemDefinition->GetAmmoWorldItemDefinition_BP(), WeaponItemDefinition->GetInitialAmmo(0), 0));
			}

			Count -= CurrentCountForEntry;
		}
	}
}

TArray<FFortItemEntry> FortKismetLibrary::PickLootDrops(FName TierGroupName, int WorldLevel)
{
	if (LootTierDatas.size() == 0 || LootPackageDatas.size() == 0)
		SetupLTDS();
	TArray<FFortItemEntry> Result;
	std::vector<FFortLootTierData*> LootTiers;
	for (auto Data : LootTierDatas)
	{
		if (!Data)
			continue;

		for (auto Pair : Data->RowMap)
		{
			auto LootTierData = reinterpret_cast<FFortLootTierData*>(Pair.Second);
			if (LootTierData) {
				if (LootTierData->TierGroup == TierGroupName)
					LootTiers.push_back(LootTierData);
			}
		}
	}
	auto LootTierData = PickWeighted<FFortLootTierData>(LootTiers);
	if (!LootTierData)
		return Result;

	float NumLootPackageDrops = LootTierData->NumLootPackageDrops;
	float NumberLootDrops = 0;

	if (NumLootPackageDrops > 0)
	{
		if (NumLootPackageDrops < 1)
		{
			NumberLootDrops = 1;
		}
		else
		{
			NumberLootDrops = (int)(float)((float)(NumLootPackageDrops + NumLootPackageDrops) - 0.5f) >> 1;
			float v20 = NumLootPackageDrops - NumberLootDrops;
			if (v20 > 0.0000099999997f)
			{
				NumberLootDrops += v20 >= (rand() * 0.000030518509f);
			}
		}
	}
	float AmountOfLootPackageDrops = GetAmountOfLootPackagesToDrop(LootTierData, NumberLootDrops);

	if (AmountOfLootPackageDrops > 0)
	{
		for (int i = 0; i < AmountOfLootPackageDrops; ++i)
		{
			if (i >= LootTierData->LootPackageCategoryMinArray.Num())
				break;

			for (int j = 0; j < LootTierData->LootPackageCategoryMinArray[i]; ++j)
			{
				if (LootTierData->LootPackageCategoryMinArray[i] < 1)
					break;

				PickLootFromLP(Result, LootTierData->LootPackage, i, WorldLevel);
			}
		}
	}
	return Result;
}

AFortPickupAthena* FortKismetLibrary::SpawnPickup(FVector Loc, UFortItemDefinition* Def, EFortPickupSourceTypeFlag SourceTypeFlag, EFortPickupSpawnSource SpawnSource, int Count, int LoadedAmmo, AFortPawn* Pawn, TArray<FFortWeaponModSlot> WeaponSlots) {
	FTransform Transform{};
	Transform.Translation = Loc;
	Transform.Scale3D = FVector(1, 1, 1);
	AFortPickupAthena* Pickup = Cast<AFortPickupAthena>(UGameplayStatics::FinishSpawningActor(UGameplayStatics::BeginDeferredActorSpawnFromClass(UWorld::GetWorld(), AFortPickupAthena::StaticClass(), (FTransform&)Transform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, nullptr, ESpawnActorScaleMethod::SelectDefaultAtRuntime), (FTransform&)Transform, ESpawnActorScaleMethod::SelectDefaultAtRuntime));
	if (!Pickup)
		return nullptr;
	Pickup->bRandomRotation = true;
	if (Pawn)
		Pickup->PawnWhoDroppedPickup = Pawn;
	Pickup->PrimaryPickupItemEntry.ItemDefinition = Def;
	Pickup->PrimaryPickupItemEntry.Count = Count;
	Pickup->PrimaryPickupItemEntry.LoadedAmmo = LoadedAmmo;
	Pickup->OnRep_PrimaryPickupItemEntry();
	
	Pickup->TossPickup(Loc, nullptr, -1, true, false, SourceTypeFlag, SpawnSource);

	if (SourceTypeFlag == EFortPickupSourceTypeFlag::Container)
	{
		Pickup->bTossedFromContainer = true;
		Pickup->OnRep_TossedFromContainer();
	}
	Pickup->PrimaryPickupItemEntry.WeaponModSlots = WeaponSlots;
	Pickup->MovementComponent = (UFortProjectileMovementComponent*)UGameplayStatics::SpawnObject(UFortProjectileMovementComponent::StaticClass(), Pickup);
	Pickup->SetReplicateMovement(true);
	Pickup->OnRep_ReplicateMovement();
	return Pickup;
}

AFortPickupAthena* FortKismetLibrary::SpawnPickup(FVector Loc, FFortItemEntry Entry, EFortPickupSourceTypeFlag SourceTypeFlag, EFortPickupSpawnSource SpawnSource, AFortPlayerPawn* Pawn, int OverrideCount, bool Toss, bool RandomRotation, bool bCombine)
{
	static auto PickupClass = AFortPickupAthena::StaticClass();
	AFortPickupAthena* NewPickup = UWorld::GetWorld()->SpawnActor<AFortPickupAthena>(PickupClass, Loc);

	NewPickup->bRandomRotation = RandomRotation;
	NewPickup->PrimaryPickupItemEntry.ItemDefinition = Entry.ItemDefinition;
	NewPickup->PrimaryPickupItemEntry.LoadedAmmo = Entry.LoadedAmmo;
	NewPickup->PrimaryPickupItemEntry.Count = OverrideCount != -1 ? OverrideCount : Entry.Count;
	NewPickup->PawnWhoDroppedPickup = Pawn;
	NewPickup->PrimaryPickupItemEntry.WeaponModSlots = Entry.WeaponModSlots;
	if (auto Weapon = Cast<UFortWeaponRangedItemDefinition>(Entry.ItemDefinition)) {
		auto Modset = Entry.GetWeaponModsetData()!=nullptr ? Entry.GetWeaponModsetData() : FortWeaponModFunctionLibrary::GetRandomModsetForWeapon(Weapon);
		if (Modset) {
			ApplyWeaponModset(NewPickup, Modset);
		}
		for (auto Mod : NewPickup->PrimaryPickupItemEntry.WeaponModSlots)
		{
			if (Mod.WeaponMod->IsA(UFortWeaponModItemDefinitionMagazine::StaticClass()))
			{
				auto tag = Mod.WeaponMod->WeaponModTag;

				auto result = Mod.WeaponMod->WeaponModDataTable->Search([&tag](FName& key, uint8*& value) {
					return key.ToString().contains(tag.TagName.ToString().c_str());
					});

				// without that if statement it crashed 
				if (result) {
					auto ModInfo = (FFortWeaponModDataRow*)(*result);
					NewPickup->PrimaryPickupItemEntry.LoadedAmmo = NewPickup->PrimaryPickupItemEntry.LoadedAmmo * ModInfo->WeaponAmmoClipSize;
				}
			}
		}
		if (!IsModSetEmpty(Modset))
		{
			FFortItemEntryStateValueConstObject NewState;
	        NewState.ObjectData = Modset;
	        FGameplayTag ModsetTag = *(FGameplayTag*)(ImageBase + 0x12BCBA54);
			UE_LOG(LogWeaponMods, Log, ModsetTag.TagName.ToString().c_str());
			NewState.StateType = ModsetTag;
			NewPickup->PrimaryPickupItemEntry.StateValuesConstObject.Add(NewState);
		}
	}
	NewPickup->OnRep_PrimaryPickupItemEntry();

	if (Toss) NewPickup->TossPickup(Loc, Pawn, -1, Toss, true, SourceTypeFlag, SpawnSource);

	if (SourceTypeFlag == EFortPickupSourceTypeFlag::Container) {
		NewPickup->bTossedFromContainer = true;
		NewPickup->OnRep_TossedFromContainer();
	}

	NewPickup->MovementComponent = (UFortProjectileMovementComponent*)UGameplayStatics::SpawnObject(UFortProjectileMovementComponent::StaticClass(), NewPickup);
	NewPickup->SetReplicateMovement(true);
	NewPickup->OnRep_ReplicateMovement();
	return NewPickup;
}

bool FortKismetLibrary::PickLootDropsHook(UObject* Object, FFrame& Stack, bool* Ret)
{
	UObject* WorldContextObject;
	FName TierGroupName;
	int32 WorldLevel;
	int32 ForcedLootTier;
	FGameplayTagContainer Container;

	Stack.StepCompiledIn(&WorldContextObject);
	auto& OutLootToDrop = Stack.StepCompiledInRef<TArray<FFortItemEntry>>();
	Stack.StepCompiledIn(&TierGroupName);
	Stack.StepCompiledIn(&WorldLevel);
	Stack.StepCompiledIn(&ForcedLootTier);
	Stack.StepCompiledIn(&Container);
	Stack.IncrementCode();

	auto LootDrops = PickLootDrops(TierGroupName, WorldLevel);

	for (auto& LootDrop : LootDrops)
		OutLootToDrop.Add(LootDrop);

	return *Ret = true;
}
AFortPickup* FortKismetLibrary::K2_SpawnPickupInWorldWithClassHook(UObject* Context, FFrame& Stack, AFortPickup** Ret)
{
	UObject* WorldContextObject;
	UFortWorldItemDefinition* ItemDefinition;
	TSubclassOf<UClass> PickupClass;
	int NumberToSpawn;
	FVector Position; 
	FVector Direction;                                              
	int OverrideMaxStackCount;                                  
	bool bToss;                                                   
	bool bRandomRotation;                                       
	bool bBlockedFromAutoPickup;                                
	int  PickupInstigatorHandle;                                 
	EFortPickupSourceTypeFlag SourceType;                                             
	EFortPickupSpawnSource Source;                                                  
	AFortPlayerController* OptionalOwnerPC;                                        
	bool bPickupOnlyRelevantToOwner;                           

	Stack.StepCompiledIn(&WorldContextObject);
	Stack.StepCompiledIn(&ItemDefinition);
	Stack.StepCompiledIn(&PickupClass);
	Stack.StepCompiledIn(&NumberToSpawn);
	Stack.StepCompiledIn(&Position);
	Stack.StepCompiledIn(&Direction);
	Stack.StepCompiledIn(&OverrideMaxStackCount);
	Stack.StepCompiledIn(&bToss);
	Stack.StepCompiledIn(&bRandomRotation);
	Stack.StepCompiledIn(&bBlockedFromAutoPickup);
	Stack.StepCompiledIn(&PickupInstigatorHandle);
	Stack.StepCompiledIn(&SourceType);
	Stack.StepCompiledIn(&Source);
	Stack.StepCompiledIn(&OptionalOwnerPC);
	Stack.StepCompiledIn(&bPickupOnlyRelevantToOwner);

	auto NewPickup = SpawnPickup(Position, ConstructItemEntry(ItemDefinition, NumberToSpawn, 0), SourceType, Source, OptionalOwnerPC ? OptionalOwnerPC->MyFortPawn : nullptr, -1, true, true, true);
	*Ret = NewPickup;
	return *Ret;
}
AFortPickup* FortKismetLibrary::K2_SpawnPickupInWorldHook(UObject* Object, FFrame& Stack, AFortPickup** Ret)
{
	class UObject* WorldContextObject;
	class UFortWorldItemDefinition* ItemDefinition;
	int32 NumberToSpawn;
	FVector Position;
	FVector Direction;
	int32 OverrideMaxStackCount;
	bool bToss;
	bool bRandomRotation;
	bool bBlockedFromAutoPickup;
	int32 PickupInstigatorHandle;
	EFortPickupSourceTypeFlag SourceType;
	EFortPickupSpawnSource Source;
	class AFortPlayerController* OptionalOwnerPC;
	bool bPickupOnlyRelevantToOwner;
	Stack.StepCompiledIn(&WorldContextObject);
	Stack.StepCompiledIn(&ItemDefinition);
	Stack.StepCompiledIn(&NumberToSpawn);
	Stack.StepCompiledIn(&Position);
	Stack.StepCompiledIn(&Direction);
	Stack.StepCompiledIn(&OverrideMaxStackCount);
	Stack.StepCompiledIn(&bToss);
	Stack.StepCompiledIn(&bRandomRotation);
	Stack.StepCompiledIn(&bBlockedFromAutoPickup);
	Stack.StepCompiledIn(&PickupInstigatorHandle);
	Stack.StepCompiledIn(&SourceType);
	Stack.StepCompiledIn(&Source);
	Stack.StepCompiledIn(&OptionalOwnerPC);
	Stack.StepCompiledIn(&bPickupOnlyRelevantToOwner);
	Stack.IncrementCode();

	return *Ret = SpawnPickup(Position, ConstructItemEntry(ItemDefinition, NumberToSpawn, 0), SourceType, Source, OptionalOwnerPC ? OptionalOwnerPC->MyFortPawn : nullptr, -1, true, true, true);
}

AFortPickup* FortKismetLibrary::SpawnItemVariantPickupInWorldHook(UObject* Object, FFrame& Stack, AFortPickup** Ret)
{
	UObject* WorldContextObject;
	FSpawnItemVariantParams Params;
	Stack.StepCompiledIn(&WorldContextObject);
	Stack.StepCompiledIn(&Params);
	Stack.IncrementCode();
	
	auto Pickup = SpawnPickup(Params.Position, ConstructItemEntry(Params.WorldItemDefinition, Params.NumberToSpawn, 0), Params.SourceType, Params.Source, Params.OptionalOwnerPC ? Params.OptionalOwnerPC->MyFortPawn : nullptr, -1, true, true, true);
	*Ret = Pickup;
	return Pickup;
}

AFortPickup* FortKismetLibrary::K2_SpawnPickupInWorldWithClassAndItemEntry(UObject* Context, FFrame& Stack, AFortPickup** Ret)
{
	UObject* WorldContextObject;
	FFortItemEntry Entry;
	TSubclassOf<AFortPickup> PickupClass;
	FVector Position;
	FVector Direction;
	int32 OverrideMaxStackCount;
	bool bToss;
	bool bRandomRotation;
	bool bBlockedFromAutoPickup;
	EFortPickupSourceTypeFlag SourceType;
	EFortPickupSpawnSource Source;
	class AFortPlayerController* OptionalOwnerPC;
	bool bPickupOnlyRelevantToOwner;

	Stack.StepCompiledIn(&WorldContextObject);
	Stack.StepCompiledIn(&Entry);
	Stack.StepCompiledIn(&PickupClass); // random ahh crash here sometimes
	Stack.StepCompiledIn(&Position);
	Stack.StepCompiledIn(&Direction);
	Stack.StepCompiledIn(&OverrideMaxStackCount);
	Stack.StepCompiledIn(&bToss);
	Stack.StepCompiledIn(&bRandomRotation);
	Stack.StepCompiledIn(&bBlockedFromAutoPickup);
	Stack.StepCompiledIn(&SourceType);
	Stack.StepCompiledIn(&Source);
	Stack.StepCompiledIn(&OptionalOwnerPC);
	Stack.StepCompiledIn(&bPickupOnlyRelevantToOwner);
	Stack.IncrementCode();

	return *Ret = SpawnPickup(Position, ConstructItemEntry((UFortItemDefinition*)Entry.ItemDefinition, Entry.Count, Entry.Level), SourceType, Source, OptionalOwnerPC ? OptionalOwnerPC->MyFortPawn : nullptr, -1, true, true, true);
}
TArray<AFortPickup*> K2_SpawnPickupInWorldWithLootTier(UObject* Context, FFrame& Parrams, TArray<AFortPickup*>* Return) {
	std::cout << "=========================================================================================================\n";
	return {};
}



void (*SetIsDoorOpen)(ABuildingWall*, uint8_t, AFortPlayerPawnAthena*) = decltype(SetIsDoorOpen)(ImageBase + 0x93ECB58);
bool FortKismetLibrary::OpenActorInternal(AActor* OpenableInterfaceActor, AFortPlayerControllerAthena* OptionalControllerInstigator,bool bRequestFastOpen)
{

	if (auto Wall = (ABuildingWall*)OpenableInterfaceActor)
	{
		auto pawn = (AFortPlayerPawnAthena*)OptionalControllerInstigator;
		SetIsDoorOpen(Wall, bRequestFastOpen, pawn);
	}
	return OpenActorInternalOG(OpenableInterfaceActor, OptionalControllerInstigator, bRequestFastOpen);
}
// int32 K2_RemoveItemFromPlayer(class AFortPlayerController* PlayerController, const class UFortWorldItemDefinition* ItemDefinition, const struct FGuid& ItemVariantGuid, int32 AmountToRemove, bool bForceRemoval)
void FortKismetLibrary::K2_RemoveItemFromPlayer(UObject* Object, FFrame& Params, int32* Ret)
{

	AFortPlayerController* PlayerController;
	UFortWorldItemDefinition* ItemDefinition;
	int32 AmountToRemove;
	bool bForceRemoval;
	
	Params.StepCompiledIn(&PlayerController);
	Params.StepCompiledIn(&ItemDefinition);
	FGuid& ItemVariantGuid = Params.StepCompiledInRef<FGuid>();
	Params.StepCompiledIn(&AmountToRemove);
	Params.StepCompiledIn(&bForceRemoval);
	Params.IncrementCode();


	auto Inventory = (AFortInventory*)PlayerController->WorldInventory.ObjectPointer;
	auto ItemEntry = Inventory->GetItemEntryByDef(ItemDefinition);
	if (ItemEntry)
	{
		Inventory->Remove(ItemEntry->ItemGuid);
	}
	
	std::cout << "K2_RemoveItemFromPlayer Called" << std::endl;
	
	
	*Ret = AmountToRemove;
}
// rewrite this asap
int32 FortKismetLibrary::K2_RemoveFortItemFromPlayer(UObject* Object, FFrame& Params, int32* Ret) 
{
	AFortPlayerController* PlayerController;
	UFortItem* Item;
	int32 AmountToRemove;
	bool bForceRemoval;
	
	Params.StepCompiledIn(&PlayerController);
	Params.StepCompiledIn(&Item);
	Params.StepCompiledIn(&AmountToRemove);
	Params.StepCompiledIn(&bForceRemoval);
	Params.IncrementCode();

	auto Inventory = (AFortInventory*)PlayerController->WorldInventory.ObjectPointer;
	auto Guid = Item->GetItemGuid();
	if (Item)
	{
		Inventory->Remove(Guid);
	}
	*Ret = AmountToRemove;
	return 1;
}

UFortWorldItem* FortKismetLibrary::GiveItemToInventoryOwner(UObject* Object, FFrame& Params)
{
	TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner;
	const UFortWorldItemDefinition* ItemDefinition;
	int32 NumberToGive;
	bool bNotifyPlayer; // aka item toast
	int32 ItemLevel; // dont care about this
	int32 PickupInstigatorHandle; // same
	bool bUseItemPickupAnalyticEvent;
	int32 WeaponAmmoOverride;

	Params.StepCompiledIn(&InventoryOwner);
	Params.StepCompiledIn(&ItemDefinition);
	FGuid& ItemVariantGuid = Params.StepCompiledInRef<FGuid>();
	Params.StepCompiledIn(&NumberToGive);
	Params.StepCompiledIn(&bNotifyPlayer);
	Params.StepCompiledIn(&ItemLevel);
	Params.StepCompiledIn(&PickupInstigatorHandle);
	Params.StepCompiledIn(&bUseItemPickupAnalyticEvent);
	Params.StepCompiledIn(&WeaponAmmoOverride);
	Params.IncrementCode();

	const AFortPawn* Pawn = ((const AFortPawn * (*)(void*))(*(void***)InventoryOwner.InterfacePointer)[9])(InventoryOwner.InterfacePointer);
	auto PlayerController = (AFortPlayerControllerAthena*)Pawn->Controller;
	auto Inventory = (AFortInventory*)PlayerController->WorldInventory.ObjectPointer;
	auto CastedDef = (UFortItemDefinition*)ItemDefinition;

	auto FoundEntry = Inventory->GetItemEntryByDef(CastedDef);
	if (FoundEntry)
	{
		FoundEntry->Count += NumberToGive;
		Inventory->ReplaceEntry(*FoundEntry);
		auto ReturnItem = Inventory->GetWorldItemByGuid(FoundEntry->ItemGuid);
		if (ReturnItem)
			return *ReturnItem;
		else
			return nullptr;
	}
	else
	{
		auto AddedItem = Inventory->AddItem(CastedDef, NumberToGive, 1, ItemLevel);
		auto ReturnItem = Inventory->GetWorldItemByGuid(AddedItem->ItemGuid);
		if (ReturnItem)
			return *ReturnItem;
		else
			return nullptr;
	}
}
bool TeleportPlayerPawn(UObject* Object, FFrame& Params)
{
	const UObject* WorldContextObject;
	AFortPlayerPawn* PlayerPawn;
	bool bIgnoreCollision;
	bool bIgnoreSupplementalKillVolumeSweep;
	Params.StepCompiledIn(&WorldContextObject);
	Params.StepCompiledIn(&PlayerPawn);
	FVector& DestLocation = Params.StepCompiledInRef<FVector>();
	FRotator& DestRotation = Params.StepCompiledInRef<FRotator>();
	Params.StepCompiledIn(&bIgnoreCollision);
	Params.StepCompiledIn(&bIgnoreSupplementalKillVolumeSweep);
	Params.IncrementCode();

	if (!PlayerPawn) return false;
	if (DestLocation.IsZero()) return false;

	
	return PlayerPawn->K2_TeleportTo(DestLocation, DestRotation);
}

void FortKismetLibrary::Init()
{
	UHook* Hook = new UHook();

	Hook->Path = "/Script/FortniteGame.FortKismetLibrary.PickLootDrops";
	Hook->Detour = PickLootDropsHook;
	UKismetHookingLibrary::Hook(Hook, Exec);

	Hook->Path = "/Script/FortniteGame.FortKismetLibrary.SpawnItemVariantPickupInWorld";
	Hook->Detour = SpawnItemVariantPickupInWorldHook;
	UKismetHookingLibrary::Hook(Hook, Exec);

	Hook->Path = "/Script/FortniteGame.FortKismetLibrary.K2_SpawnPickupInWorldWithClass";
	Hook->Detour = K2_SpawnPickupInWorldWithClassHook;
	UKismetHookingLibrary::Hook(Hook, Exec);

	Hook->Path = "/Script/FortniteGame.FortKismetLibrary.K2_SpawnPickupInWorld";
	Hook->Detour = K2_SpawnPickupInWorldHook;
	UKismetHookingLibrary::Hook(Hook, Exec);

	Hook->Path = "/Script/FortniteGame.FortKismetLibrary.K2_SpawnPickupInWorldWithClassAndItemEntry";
	Hook->Detour = K2_SpawnPickupInWorldWithClassAndItemEntry;
	UKismetHookingLibrary::Hook(Hook, Exec);

	Hook->Path = "/Script/FortniteGame.FortKismetLibrary.K2_SpawnPickupInWorldWithLootTier";
	Hook->Detour = K2_SpawnPickupInWorldWithLootTier;
	UKismetHookingLibrary::Hook(Hook, Exec);

	// Remove Item Hooks
	Hook->Path = "/Script/FortniteGame.FortKismetLibrary.K2_RemoveFortItemFromPlayer";
	Hook->Detour = K2_RemoveFortItemFromPlayer;
	UKismetHookingLibrary::Hook(Hook, Exec);
		
	Hook->Path = "/Script/FortniteGame.FortKismetLibrary.K2_RemoveItemFromPlayer";
	Hook->Detour = K2_RemoveItemFromPlayer;
	UKismetHookingLibrary::Hook(Hook, Exec);

    // all give item hooks
	Hook->Path = "/Script/FortniteGame.FortKismetLibrary.GiveItemToInventoryOwner";
	Hook->Detour = GiveItemToInventoryOwner;
	UKismetHookingLibrary::Hook(Hook, Exec);

	Hook->Path = "/Script/FortniteGame.FortMissionLibrary.TeleportPlayerPawn";
	Hook->Detour = TeleportPlayerPawn;
	UKismetHookingLibrary::Hook(Hook, Exec);
	
	Hook->Address = ImageBase + 0x8C39A84;
	Hook->Detour = OpenActorInternal;
	Hook->Original = (void**)&OpenActorInternalOG;
	UKismetHookingLibrary::Hook(Hook, Address); 

	delete Hook;
}