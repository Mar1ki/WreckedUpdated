#pragma once
#include "pch.h"

namespace FortAthenaBTService_PickUpLoot {
	static void AssessLootInteractionInfo(UFortAthenaBTService_PickUpLoot* Service, bool* bOutSkipInteractionExecution, UBlackboardComponent* BBComp, AFortAthenaAIBotController* BotController, AActor* Actor);
	DefineOriginal(bool, HasMeleeWeapon, AFortAthenaAIBotController* Controller);
	DefineOriginal(bool, HasRangedWeaponWithAmmo, AFortAthenaAIBotController* Controller, uint8 a2);
	DefineOriginal(bool, CanDoRangeAttackPositioning, UFortAthenaAIBotEvaluator_RangeAttack* AiService, UBlackboardComponent* BBComp, UFortWorldItem** OutBestWeapon, const bool bShouldConsiderLoF);
	DefineOriginal(AFortWeapon*, EquipWeapon, AFortAthenaAIBotController* Controller, UFortWorldMultiItem* Weapon);
	DefineOriginal(void, ManageWeapon, UFortAthenaBTService_ManageWeapon* ManageWeapon, UBehaviorTreeComponent* OwnerComp);
	DefineOriginal(EExecutionStatus,SetExecutionStatus, UBlackboardComponent* BBComp, unsigned __int8 BlackboardKey, EExecutionStatus Status);
	void Init();
}