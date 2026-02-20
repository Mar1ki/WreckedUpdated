#pragma once
#include "pch.h"

namespace AthenaAIServicePlayerBots {
	static void InitializeMMRInfos();
	DefineOriginal(char, CheckForBotBrainActivation, UAthenaAIServicePlayerBots* AiService);
	DefineOriginal(void, InitializeForWorld, UNavigationSystemV1* Nav,UWorld* World, EFNavigationSystemRunMode Mode);
	DefineOriginal(void, CreateAndConfigureNavigationSystem, UAthenaNavSystemConfig* ModuleConfig, UWorld* World);
	DefineOriginal(void, UpdateControlRotations, UAthenaAIServicePlayerBots* AiService, double Delta);
	DefineOriginal(APawn*, SpawnAI, UAthenaAISpawner* Spawner, UFortAthenaAISpawnerDataComponentList* ComponentList, FTransform* Transform, FActorSpawnParameters* SpawnParams, int SpawnRequestId);
	DefineOriginal(void, PerceptionTickComponent, UFortNearbyActorsPerceptionComponent* Comp, double Delta);
	void Init();
}