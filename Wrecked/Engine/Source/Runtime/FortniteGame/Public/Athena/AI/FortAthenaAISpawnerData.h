#pragma once
#include "pch.h"	

namespace FortAthenaAISpawnerData {
	static void AIBotSkillsetOnSpawned(UFortAthenaAISpawnerDataComponent_AIBotSkillset* AIBotSkillset, AFortAIPawn* Pawn);
	DefineOriginal(void, AIBotBehaviourOnSpawned, UFortAthenaAISpawnerDataComponent_Behavior* Component, AFortAIPawn* Pawn);
	static void AIBotCosmeticLoadoutOnSpawned(UFortAthenaAISpawnerDataComponent_CosmeticLoadout* Component, AFortAIPawn* Pawn);
	DefineOriginal(void, OnPawnAISpawnedHook, UAthenaAIService* Service, AAIController* Controller, AFortPawn* AIPawn);
	DefineOriginal(void, AIBotInventoryBaseOnSpawned, UFortAthenaAISpawnerDataComponent_InventoryBase* Component, AFortAIPawn* Pawn);
	DefineOriginal(void, AIBotConversationOnSpawned, UFortAthenaAISpawnerDataComponent_AIBotConversation* Component, AFortAIPawn* Pawn);
	DefineOriginal(void, sub_9A1E264,  UFortNearbyActorsPerceptionComponent* Component, float DeltaTime);

	static void* (*ApplyCharacterCustomization)(AFortPlayerStateAthena* PlayerState, APawn* Pawn) = decltype(ApplyCharacterCustomization)(ImageBase + 0xA28E8CC);
	
	void Init();
}
