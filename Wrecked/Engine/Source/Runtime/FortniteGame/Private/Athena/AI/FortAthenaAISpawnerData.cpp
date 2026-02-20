#include "pch.h"
#include "Engine/Source/Runtime/FortniteGame/Public/Athena/AI/FortAthenaAISpawnerData.h"
#include "Engine/Plugins/HookingLibrary/Public/HookingLibrary.h"
#include "Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "SDK/SDK/BP_PhoebePlayerController_classes.hpp"

void FortAthenaAISpawnerData::AIBotSkillsetOnSpawned(UFortAthenaAISpawnerDataComponent_AIBotSkillset* AIBotSkillset, AFortAIPawn* Pawn) {
	auto AIBotController = (AFortAthenaAIBotController*)Pawn->Controller;
	static UFortAthenaAIBotDigestedSkillSet* (__fastcall * GetOrCreateDigestedSkillSetInternal)(AFortAthenaAIBotController * Controller, TSubclassOf<UFortAthenaAIBotSkillSet> *InSkillSetClass, bool bForceUpdate, unsigned int* CRC) = decltype(GetOrCreateDigestedSkillSetInternal)(ImageBase + 0xA7AE888);
	if (AIBotController && AIBotController->BotSkillSetClasses.Num() == 0)
	{
		auto SkillSets = AIBotSkillset->GetSkillSets();
		for (auto SkillSet : SkillSets) {
			AIBotController->SetSkillSet(SkillSet);
			unsigned int WtfIsThis = 0;
			auto DigestSkillset = GetOrCreateDigestedSkillSetInternal(AIBotController, &SkillSet, true, &WtfIsThis);
			if (DigestSkillset) {
				unsigned int (*Digest)(UFortAthenaAIBotDigestedSkillSet*, int, UFortAthenaAIBotSkillSet*) = decltype(Digest)(__int64(DigestSkillset->VTable[0x57]));
				Digest(DigestSkillset, AIBotSkillset->Skill.Evaluate(), (UFortAthenaAIBotSkillSet*)SkillSet.Get()->GetDefaultObj());
			}
		}
	}
}

template <typename T>
T* PickWeighted(UDataTable* Table) {
	
	float TotalWeight = 0;
	for (auto Pair : Table->RowMap)
	{
		auto Element = (T*)Pair.Second;
		float Weight = Element->Weight;
		TotalWeight += 100;
	}

	float Number = (rand() * 0.000030518509f) * TotalWeight;

	for (auto Pair : Table->RowMap)
	{
		auto Element = (T*)Pair.Second;
		float Weight = Element->Weight;
		if (Weight == 0)
			continue;

		if (Number <= Weight)
			return Element;

		Number -= Weight;
	}

	return nullptr;
}

void FortAthenaAISpawnerData::AIBotBehaviourOnSpawned(UFortAthenaAISpawnerDataComponent_Behavior* Component, AFortAIPawn* Pawn) {
	AIBotBehaviourOnSpawnedOG(Component, Pawn);

	auto AIBotController = (AFortAthenaAIBotController*)Pawn->Controller;
	auto BlackBoard = AIBotController->Blackboard;
	AIBotController->BehaviorTree = Component->GetBehaviorTree();
	AIBotController->RunBehaviorTree(Component->GetBehaviorTree());//this would prob never work)
	auto Comp = (UBehaviorTreeComponent*)AIBotController->BrainComponent;
	AIBotController->UseBlackboard(Component->GetBehaviorTree()->BlackboardAsset, &BlackBoard);
	AIBotController->OnUsingBlackBoard(BlackBoard, Component->GetBehaviorTree()->BlackboardAsset);
	Comp->DefaultBehaviorTreeAsset = Component->GetBehaviorTree();
	Comp->StartLogic();
}

int GetRandomCosmeticLibrary(UFortAthenaAISpawnerDataComponent_CosmeticLibrary* Component)
{
	float TotalWeight = 0.0f;
	for (int i = 0; i < 5; i++)
		TotalWeight += Component->DefaultCosmeticLibraryWeight.EvaluateIndex((float)i);
    
	float Random = FMath::FRand() * TotalWeight;
	float Sum = 0.0f;
    
	for (int i = 0; i < 5; i++)
	{
		Sum += Component->DefaultCosmeticLibraryWeight.EvaluateIndex((float)i);
		if (Random <= Sum) return i;
	}
	return 4;
}
// npc cosmetics and bosses
void FortAthenaAISpawnerData::AIBotCosmeticLoadoutOnSpawned(UFortAthenaAISpawnerDataComponent_CosmeticLoadout* Component, AFortAIPawn* Pawn) {
	auto AIBotController = (AFortAthenaAIBotController*)Pawn->Controller;
	if (!Component || !Pawn) return;

	auto Controller = (AFortAthenaAIBotController*)Pawn->Controller;
	if (!Controller) return;

	auto BotState = (AFortPlayerStateAthena*)Controller->PlayerState;
	if (!BotState) return;

	auto Casted = (AFortPlayerPawn*)Pawn;
	if (!Casted) return;
	
	// ^^ yes we need all these checks bc it crashed randomly for me so it will not crash randomly
	FFortAthenaLoadout Loadout;
	Component->GetLoadout(&Loadout);
	
	AIBotController->CosmeticLoadoutBC.GetTyped() = Loadout;
	Casted->CosmeticLoadout = Loadout;
	BotState->OnRep_CharacterData(); 

	if (Loadout.Character)
	{
		ApplyCharacterCustomization(BotState, Casted);
		UFortKismetLibrary::UpdatePlayerCustomCharacterPartsVisualization(BotState);
	}
}

void FortAthenaAISpawnerData::AIBotPhoebeCosmeticLibraryOnSpawned(UFortAthenaAISpawnerDataComponent_CosmeticLibrary* Component, AFortAIPawn* Pawn)
{
	std::cout << "Phoebe Cosmetics Called" << std::endl;
	auto ChosenCosmeticLibrary = Component->CosmeticLibraries[GetRandomCosmeticLibrary(Component)].Get();
	std::cout << ChosenCosmeticLibrary->CharactersDataTable.Get()->Name.ToString().c_str() << std::endl;
	auto IDK = ChosenCosmeticLibrary->RetrieveDataTables(); // not stripped prob

	AIBotPhoebeCosmeticLibraryOnSpawnedOG(Component, Pawn);
}

void FortAthenaAISpawnerData::AIBotInventoryBaseOnSpawned(UFortAthenaAISpawnerDataComponent_InventoryBase* Component, AFortAIPawn* Pawn) {
	auto AIBotController = (AFortAthenaAIBotController*)Pawn->Controller;
	if (!Component || !Pawn || !AIBotController) return;
	if (!AIBotController->Inventory.ObjectPointer)
	{
		FActorSpawnParameters SpawnParams{};
		SpawnParams.NameMode = Required_Fatal;
		SpawnParams.ObjectFlags = EObjectFlags::Transactional;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.bNoFail = true;	

		FTransform PawnTransform = Pawn->GetTransform();
		AFortInventory* Object = ((AFortInventory * (*)(UWorld*, UClass*, FTransform const*, FActorSpawnParameters*))(ImageBase + 0x1A8087C))(GetWorld(), AFortInventory::StaticClass(), &PawnTransform, &SpawnParams);
		if (!Object) return;
		Object->SetOwner(Pawn);
		AIBotController->Inventory.ObjectPointer = Object;
		AIBotController->Inventory.InterfacePointer = Object->GetInterfaceAddress<IFortInventoryInterface>();
		Pawn->Inventory = Object;
		Pawn->bHasInventory = true;
	}

	auto Inventory = Pawn->Inventory;
	TArray<FItemAndCount> Items = { };
	Component->GetAllPossibleItems(&Items);
	for (auto Item : Items)
	{
		auto Casted = (UFortWeaponItemDefinition*)Item.Item;
		auto WeaponStats = Casted->GetStats();
		auto Entry = Inventory->AddItem((UFortItemDefinition*)Item.Item, Item.Count, WeaponStats->ClipSize);
		std::cout << Item.Item->Name.ToString().c_str() << std::endl;
		auto ItemInstance = Inventory->GetWorldItemByGuid(Entry->ItemGuid);

		for (auto& Slot : AIBotController->SlotItems)
		{
			if (!Slot.ItemDefinition)
			{
				Slot.ItemDefinition = Item.Item;
				Slot.WeaponDefinitionCache = Slot.ItemDefinition->IsA(UFortWeaponItemDefinition::StaticClass()) ? (UFortWeaponItemDefinition*)Item.Item: nullptr;
				Slot.BotController = AIBotController;
				Slot.FortItem = ItemInstance ? *ItemInstance : nullptr;
				break;
			}
		}
	}
	
	AIBotInventoryBaseOnSpawnedOG(Component, Pawn);
}


void FortAthenaAISpawnerData::AIBotGameplayAbilityOnSpawned(UFortAthenaAISpawnerDataComponent_GameplayAbilityBase* Component, AFortAIPawn* Pawn)
{
	AIBotGameplayAbilityOnSpawnedOG(Component, Pawn);
	auto AIBotController = (AFortAthenaAIBotController*)Pawn->Controller;
	auto ASC = Pawn->AbilitySystemComponent;
	if (!Component || !Pawn || !AIBotController || !ASC) return;
	reinterpret_cast<void(*)(void*)>(ImageBase + 0x993F158)(Pawn->GetInterfaceAddress<IAbilitySystemInterface>());
}



void FortAthenaAISpawnerData::AIBotConversationOnSpawned(UFortAthenaAISpawnerDataComponent_AIBotConversation* Component, AFortAIPawn* Pawn) {
	auto AIBotController = (AFortAthenaAIBotController*)Pawn->Controller;
	if (!Component || !Pawn || !AIBotController) return;
	
	auto ConversationParticipantComponent = (UFortNPCConversationParticipantComponent*)Pawn->GetComponentByClass(UFortNPCConversationParticipantComponent::StaticClass());
	
	if (!ConversationParticipantComponent)
	{
		ConversationParticipantComponent = (UFortNPCConversationParticipantComponent*)Pawn->AddComponentByClass(Component->ConversationComponentClass, false, FTransform(), false);
	}
	
	ConversationParticipantComponent->BotControllerOwner = AIBotController;
	ConversationParticipantComponent->PlayerPawnOwner = (AFortPlayerPawn*)Pawn;
	
	ConversationParticipantComponent->SelfParticipantTag = Component->SelfParticipantTag;
	ConversationParticipantComponent->ConversationEntryTag = Component->ConversationEntryTag;
	ConversationParticipantComponent->InteractorParticipantTag = Component->InteractorParticipantTag;

	ConversationParticipantComponent->CharacterData = Component->CharacterData;
	
	ConversationParticipantComponent->ConversationInteractionCollisionProfile = Component->ConversationInteractionCollisionProfile;
	ConversationParticipantComponent->ConversationInteractionBoxExtent = Component->ConversationInteractionBoxExtent;
	ConversationParticipantComponent->ConversationInteractionBoxOffset = Component->ConversationInteractionBoxOffset;
	ConversationParticipantComponent->ServiceProviderIDTag.TagName = Component->ConversationEntryTag.TagName;
	
	UFortAthenaNPCLoopStateComponent* LoopStateComponent = (UFortAthenaNPCLoopStateComponent*)Pawn->AddComponentByClass(Component->NPCLoopStateComponentClass, false, FTransform(), false);

	Component->UseSpecialActorComponent.Value = 1.f;
	Component->BlockSpecialActorUntilOutsideTheLoop.Value = 0.f;

	bool bUseSpecialActor = (bool)Component->UseSpecialActorComponent.Evaluate();
	bool bBlockSpecialActorUntilOutsideTheLoop = (bool)Component->BlockSpecialActorUntilOutsideTheLoop.Evaluate();
	
	LoopStateComponent->bSpawnOutsideTheLoop = (bool)Component->SpawnOutOfTheLoop.Evaluate();

	// we should actually setup special actor diffrently and make impl for ch5 
	if (bUseSpecialActor && !bBlockSpecialActorUntilOutsideTheLoop)
	{
		(UAthenaSpecialActorComponent*)Pawn->AddComponentByClass(Component->SpecialActorComponentClass, false, FTransform(), false);
	} 
	
	ConversationParticipantComponent->bCanStartConversation = true;
	ConversationParticipantComponent->OnRep_CanStartConversation();
	
	AIBotConversationOnSpawnedOG(Component, Pawn);
}


void FortAthenaAISpawnerData::Init() {
	UHook* Hook = new UHook();
	
	Hook->Address = 0x59;
	Hook->Class = UFortAthenaAISpawnerDataComponent_AIBotSkillset::StaticClass();
	Hook->Detour = AIBotSkillsetOnSpawned;
	UKismetHookingLibrary::Hook(Hook, EveryVFT);
	
	Hook->Address = 0x59;
	Hook->Class = UFortAthenaAISpawnerDataComponent_CosmeticLoadout::StaticClass();
	Hook->Detour = AIBotCosmeticLoadoutOnSpawned;
	UKismetHookingLibrary::Hook(Hook, EveryVFT);

	Hook->Address = ImageBase + 0xA8F9D74;
	Hook->Detour = AIBotBehaviourOnSpawned;
	Hook->Original = (void**)&AIBotBehaviourOnSpawnedOG;
	UKismetHookingLibrary::Hook(Hook, Address);
	
	Hook->Address = 0x59;
	Hook->Class = UFortAthenaAISpawnerDataComponent_InventoryBase::StaticClass();
	Hook->Detour = AIBotInventoryBaseOnSpawned;
	Hook->Original = (void**)&AIBotInventoryBaseOnSpawnedOG;
	UKismetHookingLibrary::Hook(Hook, EveryVFT);
	
	Hook->Address = 0x59;
	Hook->Class = UFortAthenaAISpawnerDataComponent_AIBotConversation::StaticClass();
	Hook->Detour = AIBotConversationOnSpawned;
	Hook->Original = (void**)&AIBotConversationOnSpawnedOG;
	UKismetHookingLibrary::Hook(Hook, EveryVFT);

	/*Hook->Address = 0x59;
	Hook->Class = UFortAthenaAISpawnerDataComponent_CosmeticLibrary::StaticClass();
	Hook->Detour = AIBotPhoebeCosmeticLibraryOnSpawned;
	Hook->Original = (void**)&AIBotPhoebeCosmeticLibraryOnSpawnedOG;
	UKismetHookingLibrary::Hook(Hook, EveryVFT); */

	Hook->Address = 0x59;
	Hook->Class = UFortAthenaAISpawnerDataComponent_GameplayAbilityBase::StaticClass();
	Hook->Detour = AIBotGameplayAbilityOnSpawned;
	Hook->Original = (void**)&AIBotGameplayAbilityOnSpawnedOG;
	UKismetHookingLibrary::Hook(Hook, EveryVFT);
	
	delete Hook;
}