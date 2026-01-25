#pragma once
#include "pch.h"	

namespace FortAthenaAISpawnerData {
	static void AIBotSkillsetOnSpawned(UFortAthenaAISpawnerDataComponent_AIBotSkillset* AIBotSkillset, AFortAIPawn* Pawn);
	DefineOriginal(void, AIBotBehaviourOnSpawned, UFortAthenaAISpawnerDataComponent_Behavior* AIBotSkillset, AFortAIPawn* Pawn);
	static void AIBotCosmeticLibraryOnSpawned(UFortAthenaAISpawnerDataComponent_CosmeticLibrary* AIBotSkillset, AFortAIPawn* Pawn);
	DefineOriginal(void, OnPawnAISpawnedHook, UAthenaAIService* Service, AAIController* Controller, AFortPawn* AIPawn);
	DefineOriginal(void, AIBotInventoryBaseOnSpawned, UFortAthenaAISpawnerDataComponent_InventoryBase* SpawnData, AFortAIPawn* Pawn);
	void Init();
}
