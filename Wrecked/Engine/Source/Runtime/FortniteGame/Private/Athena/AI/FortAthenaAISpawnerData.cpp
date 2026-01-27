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
	AIBotController->RunBehaviorTree(Component->GetBehaviorTree());//this would prob never work)
}

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
		
		AFortInventory* Object = ((AFortInventory * (*)(UWorld*, UClass*, FTransform const*, FActorSpawnParameters*))(ImageBase + 0x1A8087C))(GetWorld(), AFortInventory::StaticClass(), new FTransform(FVector(), {}), &SpawnParams);
		Object->SetOwner(Pawn);
		AIBotController->Inventory.ObjectPointer = Object;
		AIBotController->Inventory.InterfacePointer = Object->GetInterfaceAddress<IFortInventoryInterface>();
		Pawn->Inventory = Object;
		Pawn->bHasInventory = true;
	}
	AIBotController->StartupInventory->Items = Component->Items;
	if (AIBotController->StartupInventory)
	{
		auto Inventory = Pawn->Inventory;
		
		for (auto ItemAndCount : AIBotController->StartupInventory->Items)
		{
			int32 Ammo = 0;
			if (auto* WeaponDef = Cast<UFortWeaponItemDefinition>(ItemAndCount.Item))
				if (WeaponDef->GetStats())
					Ammo = WeaponDef->GetStats()->ClipSize;
			
			Inventory->AddItem((UFortItemDefinition*)ItemAndCount.Item, ItemAndCount.Count, Ammo);
		}
	}
	AIBotInventoryBaseOnSpawnedOG(Component, Pawn);
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


// todo make it better bc setting that cached one to the gamemode one is making npcs be dumb as hell
void FortAthenaAISpawnerData::sub_9A1E264(UFortNearbyActorsPerceptionComponent* Component, float DeltaTime)
{
	Component->CachedAIDirector = GetGameMode()->AIDirector;
	sub_9A1E264OG(Component, DeltaTime);
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


	Hook->Address = ImageBase + 0x9A1E264;
	Hook->Detour = sub_9A1E264;
	Hook->Original = (void**)&sub_9A1E264OG;
	UKismetHookingLibrary::Hook(Hook, Address);
	
	delete Hook;
}