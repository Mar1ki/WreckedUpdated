#pragma once
#include "pch.h"

namespace AthenaAIServicePlayerBots {
	static void InitializeMMRInfos();
	DefineOriginal(char, CheckForBotBrainActivation, UAthenaAIServicePlayerBots* AiService);
	DefineOriginal(void, InitializeForWorld, UNavigationSystemV1* Nav,UWorld* World, EFNavigationSystemRunMode Mode);
	DefineOriginal(void, CreateAndConfigureNavigationSystem, UAthenaNavSystemConfig* ModuleConfig, UWorld* World);
	void Init();
}