#pragma once
#include "pch.h"

namespace AthenaAIServicePlayerBots {
	static void InitializeMMRInfos();
	DefineOriginal(char, CheckForBotBrainActivation, UAthenaAIServicePlayerBots* AiService);
	void Init();
}