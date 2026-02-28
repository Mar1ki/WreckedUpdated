#pragma once
#include "pch.h"

namespace FortAthenaBTService_PickUpLoot {
	static void AssessLootInteractionInfo(UFortAthenaBTService_PickUpLoot* Service, bool* bOutSkipInteractionExecution, UBlackboardComponent* BBComp, AFortAthenaAIBotController* BotController, AActor* Actor);
	void Init();
}